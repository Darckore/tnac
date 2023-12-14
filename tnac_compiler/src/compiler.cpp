#include "compiler/compiler.hpp"
#include "common/feedback.hpp"
#include "common/diag.hpp"
#include "sema/sema.hpp"
#include "cfg/cfg.hpp"
#include "eval/value/value_registry.hpp"

namespace tnac
{
  // Special members

  compiler::~compiler() noexcept = default;

  compiler::compiler(sema& sema, ir::cfg& gr, eval::registry& reg, feedback* fb) noexcept :
    m_sema{ &sema },
    m_feedback{ fb },
    m_cfg{ &gr },
    m_valVisitor{ reg }
  {}


  // Public members

  void compiler::operator()(tree_ref node) noexcept
  {
    if (!node.is_valid())
    {
      error(diag::compilation_stopped());
      return;
    }

    base_t::operator()(&node);
  }

  const ir::cfg& compiler::cfg() const noexcept
  {
    return *m_cfg;
  }
  ir::cfg& compiler::cfg() noexcept
  {
    return FROM_CONST(cfg);
  }


  // General

  void compiler::visit(ast::root& root) noexcept
  {
    utils::unused(root);
  }

  void compiler::visit(ast::module_def& mod) noexcept
  {
    utils::unused(mod);
    m_cfg->exit_module();
  }

  void compiler::visit(ast::scope& scope) noexcept
  {
    utils::unused(scope);
  }

  void compiler::visit(ast::error_expr& err) noexcept
  {
    // Should never even be here since we break at the first module with errors
    UTILS_ASSERT(false);
    error(err.pos().at(), err.message());
  }

  // Exprs

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
    utils::unused(lit);
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

  void compiler::visit(ast::func_decl& func) noexcept
  {
    utils::unused(func);
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
      m_modules.store(modSym, *mod);
      m_modules.push(modSym);
      if (ok = walk_imports(*mod); !ok)
        break;
    }

    if (ok)
      compile_modules();

    m_modules.wipe();
    return false;
  }

  bool compiler::preview(ast::module_def& mod) noexcept
  {
    m_cfg->enter_module(mod.symbol());
    return true;
  }


  // Private members

  void compiler::compile_modules() noexcept
  {
    while (auto mod = m_modules.pop())
    {
      auto def = m_modules.locate(*mod);
      UTILS_ASSERT(def);
      utils::unused(def);
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

      m_modules.push(*modSym);
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