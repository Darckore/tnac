#include "compiler/compiler.hpp"
#include "common/feedback.hpp"
#include "common/diag.hpp"
#include "sema/sema.hpp"
#include "cfg/cfg.hpp"
#include "eval/value/value_registry.hpp"

namespace tnac::eval::detail
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

namespace tnac::detail
{
  constexpr auto to_binary_opcode(tok_kind tk) noexcept
  {
    using enum tok_kind;
    switch (tk)
    {
    case Plus:      return ir::op_code::Add;
    case Minus:     return ir::op_code::Sub;
    case Asterisk:  return ir::op_code::Mul;
    case Slash:     return ir::op_code::Div;
    case Percent:   return ir::op_code::Mod;
    case Amp:       return ir::op_code::And;
    case Pipe:      return ir::op_code::Or;
    case Hat:       return ir::op_code::Xor;
    case Pow:       return ir::op_code::Pow;
    case Root:      return ir::op_code::Root;
    case Less:      return ir::op_code::CmpL;
    case LessEq:    return ir::op_code::CmpLE;
    case Greater:   return ir::op_code::CmpG;
    case GreaterEq: return ir::op_code::CmpGE;
    case Eq:        return ir::op_code::CmpE;
    case NotEq:     return ir::op_code::CmpNE;

    default: break;
    }

    return ir::op_code::None;
  }
  constexpr auto to_unary_opcode(tok_kind tk) noexcept
  {
    using enum tok_kind;
    switch (tk)
    {
    case Exclamation: return ir::op_code::CmpNot;
    case Question:    return ir::op_code::CmpIs;
    case Plus:        return ir::op_code::Plus;
    case Minus:       return ir::op_code::Neg;
    case Tilde:       return ir::op_code::BNeg;

    default: break;
    }

    return ir::op_code::None;
  }
  constexpr auto is_land(tok_kind tk) noexcept
  {
    return tk == tok_kind::LogAnd;
  }
  constexpr auto is_lor(tok_kind tk) noexcept
  {
    return tk == tok_kind::LogOr;
  }
  constexpr auto is_logical(tok_kind tk) noexcept
  {
    return is_land(tk) || is_lor(tk);
  }
  constexpr auto logical_to_str(tok_kind tk) noexcept
  {
    switch (tk)
    {
    case tok_kind::LogAnd: return "land"sv;
    case tok_kind::LogOr:  return "lor"sv;
    default: break;
    }
    return string_t{};
  }
  constexpr auto is_assign(tok_kind tk) noexcept
  {
    return utils::eq_any(tk, tok_kind::Assign);
  }

  template <typename T>
  constexpr auto expected_args() -> std::pair<std::size_t, std::size_t>
  {
    using ti = eval::type_info<T>;
    return { ti::minArgs, ti::maxArgs };
  }

  auto expected_args(const token& tk) noexcept -> std::pair<std::size_t, std::size_t>
  {
    using detail::expected_args;
    using enum tok_kind;

    switch (tk.what())
    {
    case KwComplex:  return expected_args<eval::complex_type>();
    case KwFraction: return expected_args<eval::fraction_type>();
    case KwInt:      return expected_args<eval::int_type>();
    case KwFloat:    return expected_args<eval::float_type>();
    case KwBool:     return expected_args<eval::bool_type>();

    default: UTILS_ASSERT(false); break;
    }
    return {};
  }
  auto to_type_id(const token& tk) noexcept
  {
    using enum tok_kind;

    switch (tk.what())
    {
    case KwComplex:  return eval::type_id::Complex;
    case KwFraction: return eval::type_id::Fraction;
    case KwInt:      return eval::type_id::Int;
    case KwFloat:    return eval::type_id::Float;
    case KwBool:     return eval::type_id::Bool;

    default: UTILS_ASSERT(false); break;
    }
    return eval::type_id::Invalid;
  }

  constexpr auto to_inst_code(eval::type_id ti) noexcept
  {
    using enum eval::type_id;
    switch (ti)
    {
    case Bool:     return ir::op_code::Bool;
    case Int:      return ir::op_code::Int;
    case Float:    return ir::op_code::Float;
    case Fraction: return ir::op_code::Frac;
    case Complex:  return ir::op_code::Cplx;

    default: UTILS_ASSERT(false); break;
    }

    return ir::op_code::None;
  }

  constexpr auto needs_named_reg(ir::op_code oc) noexcept
  {
    using enum ir::op_code;
    return utils::eq_none(oc, Load, Phi, Bool, Int, Float, Frac, Cplx);
  }

  template <typename F>
  concept fv_callback = std::is_nothrow_invocable_v<F, ast::func_decl&>;

  template <fv_callback Func>
  class func_visitor final : public ast::bottom_up_visitor<func_visitor<Func>>
  {
  public:
    CLASS_SPECIALS_NONE(func_visitor);

    func_visitor(Func callback) noexcept :
      m_callback{ std::move(callback) }
    {}

    bool preview(ast::func_decl& fd) noexcept
    {
      if(!fd.name().starts_with('`'))
        m_callback(fd);
      return false;
    }

  private:
    Func m_callback;
  };
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

  void compiler::visit(ast::result_expr&) noexcept
  {
    if (auto last = m_context.last_store())
      emit_load(*last);
    if (m_stack.empty())
      m_stack.push_undef();
    else
      m_stack.push(m_stack.top());
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
    emit_load(id.symbol());
  }

  void compiler::visit(ast::unary_expr& unary) noexcept
  {
    auto val = extract();
    const auto opType = unary.op().what();
    compile_unary(&unary, val, opType);
  }

  void compiler::visit(ast::binary_expr& binary) noexcept
  {
    const auto opType = binary.op().what();
    if (detail::is_logical(opType) || detail::is_assign(opType))
      return;

    auto rhs = extract();
    auto lhs = extract();
    compile_binary(&binary, lhs, rhs, opType);
  }

  void compiler::visit(ast::array_expr& arr) noexcept
  {
    utils::unused(arr);
  }

  void compiler::visit(ast::abs_expr& abs) noexcept
  {
    auto val = extract();
    if (val.is_value())
    {
      m_eval.visit_unary(val.get_value(), eval::val_ops::AbsoluteValue);
      carry_val(&abs);
      return;
    }

    emit_unary(ir::op_code::Abs, val);
  }

  void compiler::visit(ast::typed_expr& typed) noexcept
  {
    auto&& args = typed.args();
    const auto argSz = args.size();
    const auto argLimits = detail::expected_args(typed.type_name());
    if (!utils::in_range(argSz, argLimits.first, argLimits.second))
    {
      const auto threshold = argSz < argLimits.first ? argLimits.first : argLimits.second;
      error(diag::wrong_arg_num(threshold, argSz));
      m_stack.drop(argSz);
      m_stack.push_undef();
      return;
    }

    const auto typeId = detail::to_type_id(typed.type_name());
    if (m_stack.has_values(argSz))
    {
      auto count = argSz;
      while (count--)
      {
        auto val = m_stack.extract();
        m_eval.push_value(val.get_value());
      }
      m_eval.instantiate(typeId, argSz);
      carry_val(&typed);
      return;
    }

    emit_inst(detail::to_inst_code(typeId), argLimits.second, argSz);
  }

  void compiler::visit(ast::call_expr& call) noexcept
  {
    utils::unused(call);
  }

  void compiler::visit(ast::dot_expr& dot) noexcept
  {
    utils::unused(dot);
  }

  bool compiler::exit_child(ast::node& node) noexcept
  {
    return !node.is(ast::node_kind::Ret);
  }

  void compiler::post_exit(ast::node& node) noexcept
  {
    auto loc = try_get_location(node);
    if (!loc) return;
    warning(*loc, diag::unreachable());
  }

  // Decls

  void compiler::visit(ast::param_decl& param) noexcept
  {
    auto&& sym = param.symbol();
    emit_alloc(sym);
    emit_store(sym);
  }

  void compiler::visit(ast::func_decl& fd) noexcept
  {
    auto&& sym = fd.symbol();
    auto func = m_cfg->find_entity(&sym);
    UTILS_ASSERT(func);
    auto funcVal = m_eval.make_function(&fd, eval::function_type{ *func });
    m_stack.push(funcVal);
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
    auto lastVal = m_stack.try_extract();
    auto&& owner = m_context.current_function();
    const auto parCnt = fd.param_count();
    auto funcName = m_names.mangle_func_name(fd.name(), owner, parCnt);
    auto&& func = m_cfg->declare_function(&fd.symbol(), owner, funcName, parCnt);
    m_context.enter_function(func);
    compile(fd.params(), fd.body().children());
    m_context.exit_function();
    if(!lastVal.is_undef())
      m_stack.push(lastVal);
    return false;
  }

  bool compiler::preview(ast::var_decl& var) noexcept
  {
    auto&& sym = var.symbol();
    emit_alloc(sym);
    compile(var.initialiser());
    emit_store(sym);
    return false;
  }

  bool compiler::preview(ast::assign_expr& assign) noexcept
  {
    auto target = utils::try_cast<ast::id_expr>(&assign.left());
    UTILS_ASSERT(target);

    compile(assign.right());
    emit_store(target->symbol());
    return false;
  }

  bool compiler::preview(ast::binary_expr& binary) noexcept
  {
    const auto opType = binary.op().what();
    if (!detail::is_logical(opType))
      return true;

    auto alwaysSame = [&](eval::value val, bool isLhs) noexcept
      {
        const auto boolVal = eval::to_bool(val);
        const auto knownVal = ( boolVal && detail::is_lor(opType)) || // always true
                              (!boolVal && detail::is_land(opType));  // always false

        if(knownVal)
        {
          auto binOp = binary.op();
          warning(binOp.at(), diag::logical_same(binOp.value(), isLhs, boolVal));
          m_eval.visit_bool_literal(boolVal);
          carry_val(&binary);
          return true;
        }
        return false;
      };

    compile(binary.left());
    auto leftOp = extract();
    if (leftOp.is_value())
    {
      if (!alwaysSame(leftOp.get_value(), true))
      {
        compile(binary.right());
      }
      return false;
    }

    auto&& lastBlock = m_context.current_block();
    auto lastEnd = m_context.func_end();

    auto opName = detail::logical_to_str(opType);
    auto&& rhsBlock = m_context.create_block(m_names.make_block_name(opName, "rhs"sv));
    m_context.enter_block(rhsBlock);
    m_context.terminate_at(rhsBlock);
    compile(binary.right());

    auto rightOp = extract();
    if (rightOp.is_value())
    {
      m_context.enter_block(lastBlock);
      m_context.override_last(lastBlock.end());
      m_context.terminate_at(lastBlock);
      if (!alwaysSame(rightOp.get_value(), false))
      {
        m_stack.push(leftOp);
      }

      m_context.override_last(lastEnd);
      m_context.current_function().delete_block_tree(rhsBlock);
      return false;
    }

    auto&& endBlock = m_context.create_block(m_names.make_block_name(opName, "end"sv));
    lastEnd = m_context.override_last(lastBlock.end());
    m_context.enter_block(lastBlock);
    if(detail::is_lor(opType))
      emit_cond_jump(leftOp, endBlock, rhsBlock);
    else
      emit_cond_jump(leftOp, rhsBlock, endBlock);

    auto finalBlock = m_context.terminal_block();
    UTILS_ASSERT(finalBlock);
    lastEnd = m_context.override_last(finalBlock->end());
    m_context.enter_block(*finalBlock);
    emit_jump(rightOp, endBlock);

    m_context.enter_block(endBlock);
    m_context.terminate_at(endBlock);
    converge();

    return false;
  }

  bool compiler::preview(ast::cond_short& cond) noexcept
  {
    compile(cond.cond());
    auto checkedVal = extract();
    if (checkedVal.is_value())
    {
      auto boolVal = eval::to_bool(checkedVal.get_value());
      warning(cond.cond().pos().at(), diag::condition_same(boolVal));
      if (boolVal && !cond.has_true())
      {
        m_stack.push(checkedVal);
      }
      else if (!boolVal && !cond.has_false())
      {
        m_stack.push_undef();
      }
      else
      {
        auto&& condPart = boolVal ? cond.on_true() : cond.on_false();
        compile(condPart);
      }

      return false;
    }

    constexpr auto namePref = "cond";
    auto&& lastBlock = m_context.current_block();
    auto lastEnd = m_context.func_end();

    auto&& endBlock = m_context.create_block(m_names.make_block_name(namePref, "end"sv));

    m_stack.push(checkedVal);
    auto&& onTrue  = m_context.create_block(m_names.make_block_name(namePref, "true"sv));
    m_context.enter_block(onTrue);
    m_context.terminate_at(onTrue);
    if (!cond.has_true())
      m_stack.push(checkedVal);
    else
      compile(cond.on_true());
    auto trueRes = extract();
    extract();
    emit_jump(trueRes, endBlock);

    m_stack.push(checkedVal);
    auto&& onFalse = m_context.create_block(m_names.make_block_name(namePref, "false"sv));
    m_context.enter_block(onFalse);
    m_context.terminate_at(onFalse);
    if (!cond.has_false())
      m_stack.push_undef();
    else
      compile(cond.on_false());
    auto falseRes = extract();
    extract();
    emit_jump(falseRes, endBlock);

    lastEnd = m_context.override_last(lastBlock.end());
    m_context.enter_block(lastBlock);
    if (trueRes.is_value() && falseRes.is_value())
    {
      emit_select(checkedVal, trueRes, falseRes);
      auto&& curFn = m_context.current_function();
      curFn.delete_block_tree(onTrue);
      curFn.delete_block_tree(onFalse);
      m_context.terminate_at(lastBlock);
      m_context.override_last(lastEnd);
      return false;
    }

    emit_cond_jump(checkedVal, onTrue, onFalse);

    m_context.enter_block(endBlock);
    m_context.terminate_at(endBlock);
    converge();

    return false;
  }

  bool compiler::preview(ast::cond_expr& cond) noexcept
  {
    compile(cond.cond());
    auto checkedVal = extract();
    for (auto child : cond.patterns().children())
    {
      auto&& pattern = utils::cast<ast::pattern>(*child);
      if (compile(pattern, checkedVal))
        break;
    }
    return false;
  }


  // Private members (Emitions)

  void compiler::update_func_start(ir::instruction& instr) noexcept
  {
    m_context.func_start_at(instr);
  }

  void compiler::clear_store() noexcept
  {
    m_context.clear_store();
  }

  void compiler::converge() noexcept
  {
    auto preds = m_context.current_block().preds();
    if (preds.empty())
      return;

    // Corner case, some degenerate branch here
    if (preds.size() == 1)
    {
      m_stack.push(preds.front()->value());
      return;
    }

    emit_phi(preds);
  }

  ir::instruction& compiler::make(ir::op_code oc, size_opt prealloc /*= {}*/) noexcept
  {
    clear_store();
    using enum ir::op_code;

    auto&& builder = m_cfg->get_builder();
    auto&& block = m_context.current_block();
    auto&& instr = prealloc ?
      builder.add_instruction(block, oc, *prealloc, m_context.func_end()):
      builder.add_instruction(block, oc, m_context.func_end());

    auto&& res = detail::needs_named_reg(oc) ?
      builder.make_register(m_names.op_name(oc)):
      builder.make_register(m_context.register_index());

    instr.add(&res);
    update_func_start(instr);
    m_stack.push(&res);
    return instr;
  }

  void compiler::emit_alloc(semantics::symbol& sym) noexcept
  {
    auto varName = sym.name();
    auto&& curFn = m_context.current_function();
    auto&& entry = curFn.entry();
    auto&& builder = m_cfg->get_builder();
    auto&& var = builder.add_var(entry, m_context.funct_start());
    auto&& reg = builder.make_register(varName);
    var.add(&reg);
    m_context.store(sym, reg);
  }

  void compiler::emit_ret(ir::basic_block& block) noexcept
  {
    if (auto last = m_context.last_store())
      emit_load(*last);

    auto op = m_stack.extract();
    auto&& instr = m_cfg->get_builder().add_instruction(block, ir::op_code::Ret, m_context.func_end());
    instr.add(op);
    update_func_start(instr);
    empty_stack();
    m_context.exit_block();
  }

  void compiler::emit_store(semantics::symbol& var) noexcept
  {
    auto target = m_context.locate(var);
    UTILS_ASSERT(target);
    auto val = m_stack.extract();
    if(!val.is_param())
      m_context.save_store(var);

    auto&& block = m_context.current_block();
    auto&& instr = m_cfg->get_builder().add_instruction(block, ir::op_code::Store, m_context.func_end());
    instr.add(val).add(target);
    m_context.modify(var);
    update_func_start(instr);
  }

  void compiler::emit_load(semantics::symbol& var) noexcept
  {
    if (auto res = m_context.last_read(var))
    {
      clear_store();
      m_stack.push(res);
      return;
    }

    auto target = m_context.locate(var);
    UTILS_ASSERT(target);

    auto&& instr = make(ir::op_code::Load).add(target);
    auto&& res = instr[0];
    m_context.read_into(var, res.get_reg());
  }

  void compiler::emit_binary(ir::op_code oc, ir::operand lhs, ir::operand rhs) noexcept
  {
    make(oc).add(lhs).add(rhs);
  }

  void compiler::emit_unary(ir::op_code oc, ir::operand val) noexcept
  {
    make(oc).add(val);
  }

  void compiler::emit_jump(ir::operand value, ir::basic_block& dest) noexcept
  {
    clear_store();
    auto&& block = m_context.current_block();
    auto&& instr = m_cfg->get_builder().add_instruction(block, ir::op_code::Jump, m_context.func_end());
    instr.add(&dest);
    m_cfg->connect(block, dest, value);
    update_func_start(instr);
  }

  void compiler::emit_cond_jump(ir::operand cond, ir::basic_block& ifTrue, ir::basic_block& ifFalse) noexcept
  {
    clear_store();
    auto&& block = m_context.current_block();
    auto&& instr = m_cfg->get_builder().add_instruction(block, ir::op_code::Jump, m_context.func_end());
    instr.add(cond).add(&ifTrue).add(&ifFalse);
    m_cfg->connect(block, ifTrue, cond);
    m_cfg->connect(block, ifFalse, cond);
    update_func_start(instr);
  }

  void compiler::emit_phi(edge_view edges) noexcept
  {
    clear_store();
    auto&& instr = make(ir::op_code::Phi, edges.size() + 1);
    for (auto edge : edges)
    {
      instr.add(edge);
    }
  }

  void compiler::emit_select(ir::operand cond, ir::operand onTrue, ir::operand onFalse) noexcept
  {
    clear_store();
    make(ir::op_code::Select).add(cond).add(onTrue).add(onFalse);
  }

  void compiler::emit_inst(ir::op_code oc, size_type opCount, size_type factCount) noexcept
  {
    UTILS_ASSERT(factCount <= opCount);
    auto&& instr = make(oc);
    auto res = extract();
    m_stack.fill(instr, factCount);
    while (factCount < opCount)
    {
      instr.add(eval::value::zero());
      ++factCount;
    }
    m_stack.push(res);
  }

  // Private members

  ir::operand compiler::extract() noexcept
  {
    if (!m_stack.empty())
      return m_stack.extract();

    if (auto last = m_context.last_store())
    {
      emit_load(*last);
      return m_stack.extract();
    }

    return eval::value{};
  }

  void compiler::carry_val(entity_id id) noexcept
  {
    auto stored = m_eval.fetch_next();
    auto val = m_eval.visit_assign(id, *stored);
    m_stack.push(val);
  }

  void compiler::empty_stack() noexcept
  {
    while (!m_stack.empty())
    {
      // todo: warning - dead code
      m_stack.pop();
    }
  }

  void compiler::compile_unary(entity_id expr, const ir::operand& val, tok_kind opType) noexcept
  {
    if (val.is_value())
    {
      const auto op = eval::detail::conv_unary(opType);
      m_eval.visit_unary(val.get_value(), op);
      carry_val(expr);
      return;
    }

    const auto opcode = detail::to_unary_opcode(opType);
    UTILS_ASSERT(opcode != ir::op_code::None);
    emit_unary(opcode, val);
  }

  void compiler::compile_binary(entity_id expr, const ir::operand& lhs, const ir::operand& rhs, tok_kind opType) noexcept
  {
    if (lhs.is_value() && rhs.is_value())
    {
      const auto op = eval::detail::conv_binary(opType);
      m_eval.visit_binary(lhs.get_value(), rhs.get_value(), op);
      carry_val(expr);
      return;
    }

    const auto opcode = detail::to_binary_opcode(opType);
    UTILS_ASSERT(opcode != ir::op_code::None);
    emit_binary(opcode, lhs, rhs);
  }

  bool compiler::compile(ast::pattern& pattern, const ir::operand& checked) noexcept
  {
    auto&& matcher = utils::cast<ast::matcher>(pattern.matcher());
    if (matcher.is_default())
    {
      compile(pattern.body());
      return true;
    }

    const auto op = matcher.has_implicit_op() ? tok_kind::Eq : matcher.pos().what();
    if (matcher.is_unary())
    {
      compile_unary(&matcher, checked, op);
    }
    else
    {
      compile(matcher.checked());
      auto rhs = extract();
      compile_binary(&matcher, checked, rhs, op);
    }

    bool matchFound{};
    auto checkRes = extract();
    if (checkRes.is_value())
    {
      matchFound = eval::to_bool(checkRes.get_value());
      if (!matchFound)
        return false;
    }

    compile(pattern.body());
    return matchFound;
  }

  void compiler::compile(params_t& params, body_t& body) noexcept
  {
    auto _ = m_names.init_indicies();
    auto&& entry = m_context.create_block(m_names.entry_block_name());
    m_context.enter_block(entry);
    for (ir::function::size_type idx{}; auto param : params)
    {
      m_stack.push(ir::func_param{ idx++ });
      compile(*param);
    }

    compile(body);

    if (m_stack.empty())
    {
      m_stack.push_undef(); // Undefined, probably, an empty body
    }

    auto&& block = m_context.terminal_or_entry();
    emit_ret(block);
    UTILS_ASSERT(m_stack.empty());
  }

  void compiler::compile(body_t& body) noexcept
  {
    bool compileExprs = true;
    bool reportExit = false;
    ast::node* ret{};
    for (auto child : body)
    {
      if (compileExprs)
      {
        compile(*child);
      }

      if (!exit_child(*child))
      {
        ret = child;
        reportExit = true;
        compileExprs = false;
        continue;
      }

      if (compileExprs)
        continue;

      const auto hasFunc = compile_funcs(*child);
      if (!hasFunc && reportExit)
      {
        reportExit = false;
        post_exit(*child);
        if (!ret->is(ast::node_kind::Ret))
          continue;
        if (auto loc = try_get_location(*ret))
          note(std::move(*loc), diag::ret_here());
      }
    }
  }

  bool compiler::compile_funcs(ast::node& expr) noexcept
  {
    bool res = false;
    detail::func_visitor fv{ [&](ast::func_decl& fd) noexcept
      {
        preview(fd);
        res = true;
      } };

    fv(&expr);
    return res;
  }

  void compiler::compile(semantics::module_sym& mod) noexcept
  {
    // This module has already been compiled
    if (m_cfg->find_entity(&mod))
      return;

    auto def = m_context.locate(mod);
    UTILS_ASSERT(def);

    const auto parCnt = mod.param_count();
    auto modName = m_names.mangle_module_name(mod, parCnt);
    auto&& irMod = m_cfg->declare_module(&mod, modName, parCnt);
    m_context.enter_function(irMod);
    compile(def->params(), def->children());
    m_context.exit_function();
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

  void compiler::warning(src::loc_wrapper loc, string_t msg) noexcept
  {
    if (!m_feedback)
      return;

    m_feedback->compile_warning(std::move(loc), msg);
  }

  void compiler::note(src::loc_wrapper loc, string_t msg) noexcept
  {
    if (!m_feedback)
      return;

    m_feedback->compile_note(std::move(loc), msg);
  }
}