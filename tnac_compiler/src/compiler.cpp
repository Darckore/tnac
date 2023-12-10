#include "compiler/compiler.hpp"
#include "common/feedback.hpp"
#include "common/diag.hpp"
#include "sema/sema.hpp"
#include "cfg/builder.hpp"

namespace tnac
{
  // Special members

  compiler::~compiler() noexcept = default;

  compiler::compiler(sema& sema, feedback* fb) noexcept :
    m_sema{ &sema },
    m_feedback{ fb }
  {}


  // Public members

  void compiler::operator()(tree_ref node) noexcept
  {
    base_t::operator()(&node);
  }

  const ir::cfg& compiler::cfg() const noexcept
  {
    return m_cfg;
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
  }

  void compiler::visit(ast::import_dir& imp) noexcept
  {
    utils::unused(imp);
  }

  void compiler::visit(ast::scope& scope) noexcept
  {
    utils::unused(scope);
  }

  void compiler::visit(ast::error_expr& err) noexcept
  {
    utils::unused(err);
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

}