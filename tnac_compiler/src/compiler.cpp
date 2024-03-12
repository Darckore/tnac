#include "compiler/compiler.hpp"
#include "common/feedback.hpp"
#include "common/diag.hpp"
#include "sema/sema.hpp"
#include "cfg/cfg.hpp"
#include "eval/value/value_registry.hpp"

namespace tnac::detail
{
  namespace
  {
    constexpr auto conv_unary(tok_kind tk) noexcept
    {
      using enum tok_kind;
      using enum eval::val_ops;
      switch (tk)
      {
      case Exclamation: return LogicalNot;
      case Question:    return LogicalIs;
      case Plus:        return UnaryPlus;
      case Minus:       return UnaryNegation;
      case Tilde:       return UnaryBitwiseNot;

      default: return InvalidOp;
      }
    }
    constexpr auto conv_binary(tok_kind tk) noexcept
    {
      using enum tok_kind;
      using enum eval::val_ops;
      switch (tk)
      {
      case Plus:     return Addition;
      case Minus:    return Subtraction;
      case Asterisk: return Multiplication;
      case Slash:    return Division;
      case Percent:  return Modulo;

      case Less:      return RelLess;
      case LessEq:    return RelLessEq;
      case Greater:   return RelGr;
      case GreaterEq: return RelGrEq;
      case Eq:        return Equal;
      case NotEq:     return NEqual;

      case Amp:  return BitwiseAnd;
      case Hat:  return BitwiseXor;
      case Pipe: return BitwiseOr;

      case Pow:  return BinaryPow;
      case Root: return BinaryRoot;

      default: return InvalidOp;
      }
    }
  }
}

namespace tnac
{
  // Special members

  compiler::~compiler() noexcept = default;

  compiler::compiler(sema& sema, ir::cfg& gr, eval::registry& reg, feedback* fb) noexcept :
    m_sema{ &sema },
    m_feedback{ fb },
    m_cfg{ &gr },
    m_eval{ reg }
  {}


  // Public members

  void compiler::operator()(tree_ref node) noexcept
  {
    if (!node.is_valid())
    {
      error(diag::compilation_stopped());
      return;
    }

    compile(node);
  }

  const ir::cfg& compiler::cfg() const noexcept
  {
    return *m_cfg;
  }
  ir::cfg& compiler::cfg() noexcept
  {
    return FROM_CONST(cfg);
  }


  // Exprs

  void compiler::visit(ast::error_expr& err) noexcept
  {
    // Should never even be here since we break at the first module with errors
    UTILS_ASSERT(false);
    error(err.pos().at(), err.message());
  }

  void compiler::visit(ast::result_expr& res) noexcept
  {
    utils::unused(res);
  }

  void compiler::visit(ast::ret_expr& ret) noexcept
  {
    utils::unused(ret);
  }

  void compiler::visit(ast::lit_expr& lit) noexcept
  {
    auto&& litValue = lit.pos();
    switch (litValue.what())
    {
    case token::KwTrue:  m_eval.visit_bool_literal(true);                break;
    case token::KwFalse: m_eval.visit_bool_literal(false);               break;
    case token::KwI:     m_eval.visit_i();                               break;
    case token::KwPi:    m_eval.visit_pi();                              break;
    case token::KwE:     m_eval.visit_e();                               break;
    case token::IntDec:  m_eval.visit_int_literal(litValue.value(), 10); break;
    case token::IntBin:  m_eval.visit_int_literal(litValue.value(), 2);  break;
    case token::IntOct:  m_eval.visit_int_literal(litValue.value(), 8);  break;
    case token::IntHex:  m_eval.visit_int_literal(litValue.value(), 16); break;
    case token::Float:   m_eval.visit_float_literal(litValue.value());   break;

    default: return;
    }

    carry_val(&lit);
  }

  void compiler::visit(ast::id_expr& id) noexcept
  {
    utils::unused(id);
  }

  void compiler::visit(ast::unary_expr& unary) noexcept
  {
    utils::unused(unary);
  }

  void compiler::visit(ast::binary_expr& binary) noexcept
  {
    auto rhs = m_stack.extract();
    auto lhs = m_stack.extract();
    if (lhs.is_value() && rhs.is_value())
    {
      const auto op = detail::conv_binary(binary.op().what());
      m_eval.visit_binary(lhs.get_value(), rhs.get_value(), op);
      carry_val(&binary);
      return;
    }

    utils::unused(binary);
  }

  void compiler::visit(ast::assign_expr& assign) noexcept
  {
    utils::unused(assign);
  }

  void compiler::visit(ast::decl_expr& decl) noexcept
  {
    utils::unused(decl);
  }

  void compiler::visit(ast::array_expr& arr) noexcept
  {
    utils::unused(arr);
  }

  void compiler::visit(ast::paren_expr& paren) noexcept
  {
    utils::unused(paren);
  }

  void compiler::visit(ast::abs_expr& abs) noexcept
  {
    utils::unused(abs);
  }

  void compiler::visit(ast::typed_expr& typed) noexcept
  {
    utils::unused(typed);
  }

  void compiler::visit(ast::call_expr& call) noexcept
  {
    utils::unused(call);
  }

  void compiler::visit(ast::matcher& matcher) noexcept
  {
    utils::unused(matcher);
  }

  void compiler::visit(ast::pattern& pattern) noexcept
  {
    utils::unused(pattern);
  }

  void compiler::visit(ast::cond_short& cond) noexcept
  {
    utils::unused(cond);
  }

  void compiler::visit(ast::cond_expr& cond) noexcept
  {
    utils::unused(cond);
  }

  void compiler::visit(ast::dot_expr& dot) noexcept
  {
    utils::unused(dot);
  }

  // Decls

  void compiler::visit(ast::var_decl& var) noexcept
  {
    utils::unused(var);
  }

  void compiler::visit(ast::param_decl& param) noexcept
  {
    utils::unused(param);
  }

  // Previews

  bool compiler::preview(ast::root& root) noexcept
  {
    auto&& modules = root.modules();
    auto ok = true;
    for (auto mod : modules)
    {
      if (!mod->is_valid())
      {
        error(diag::compilation_stopped(mod->name()));
        ok = false;
        break;
      }

      auto&& modSym = mod->symbol();
      m_context.store(modSym, *mod);
      m_context.push(modSym);
      if (ok = walk_imports(*mod); !ok)
        break;
    }

    if (ok)
      compile_modules();

    m_context.wipe();
    return false;
  }

  bool compiler::preview(ast::func_decl& fd) noexcept
  {
    auto&& owner = m_context.current_function();
    const auto parCnt = fd.param_count();
    auto funcName = m_names.mangle_func_name(fd.name(), owner, parCnt);
    auto&& func = m_cfg->declare_function(&fd.symbol(), owner, funcName, parCnt);
    m_context.enter_function(func);
    compile(fd.params(), fd.body().children());
    m_context.exit_function();
    return false;
  }


  // Private members

  void compiler::carry_val(entity_id id) noexcept
  {
    auto stored = m_eval.fetch_next();
    auto val = m_eval.visit_assign(id, *stored);
    m_stack.push(val);
  }

  void compiler::compile(params_t& params, body_t& body) noexcept
  {
    auto _ = m_names.init_indicies();
    auto&& entry = m_context.create_block(m_names.entry_block_name());
    m_context.enqueue_block(entry);
    for (auto param : params)
    {
      compile(*param);
    }
    for (auto child : body)
    {
      compile(*child);
    }
    m_context.exit_block();
  }

  void compiler::compile(semantics::module_sym& mod) noexcept
  {
    // This module has already been compiled
    if (m_cfg->find_entity(&mod))
      return;

    auto def = m_context.locate(mod);
    UTILS_ASSERT(def);

    const auto parCnt = mod.param_count();
    auto modName = m_names.mangle_module_name(mod.name(), parCnt);
    auto&& irMod = m_cfg->declare_module(&mod, modName, parCnt);
    m_context.enter_module(irMod);
    compile(def->params(), def->children());
    m_context.exit_module();
  }

  void compiler::compile(tree_ref node) noexcept
  {
    base_t::operator()(&node);
  }

  void compiler::compile_modules() noexcept
  {
    while (auto mod = m_context.pop())
    {
      compile(*mod);
    }
  }

  semantics::module_sym* compiler::get_module(ast::import_dir& imp) noexcept
  {
    auto sym = &imp.imported_sym();
    auto modSym = utils::try_cast<semantics::module_sym>(sym);
    if (!modSym)
    {
      if (auto ref = utils::try_cast<semantics::scope_ref>(sym))
        modSym = ref->referenced().to_module();
    }

    return modSym;
  }

  bool compiler::walk_imports(ast::module_def& mod) noexcept
  {
    for (auto imp : mod.imports())
    {
      auto modSym = get_module(*imp);
      if (!modSym)
      {
        error(imp->name().back()->pos().at(), diag::undef_id());
        return false;
      }

      m_context.push(*modSym);
    }

    return true;
  }

  void compiler::error(string_t msg) noexcept
  {
    if (!m_feedback)
      return;

    m_feedback->error(msg);
  }

  void compiler::error(src::loc_wrapper loc, string_t msg) noexcept
  {
    if (!m_feedback)
      return;

    m_feedback->compile_error(std::move(loc), msg);
  }
}