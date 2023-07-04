#include "ast/ast_builder.hpp"

namespace tnac::ast
{
  // Special members

  builder::builder() noexcept = default;
  builder::~builder() noexcept = default;

  // Public members

  // General

  scope* builder::make_scope(scope::elem_list children) noexcept
  {
    return make<scope>(std::move(children));
  }

  error_expr* builder::make_error(const token& pos, string_t msg) noexcept
  {
    return make<error_expr>(pos, msg);
  }

  // Expressions

  result_expr* builder::make_result(const token& tok) noexcept
  {
    return make<result_expr>(tok);
  }

  ret_expr* builder::make_ret(expr& retVal, const token& kwPos) noexcept
  {
    return make<ret_expr>(retVal, kwPos);
  }

  lit_expr* builder::make_literal(const token& tok) noexcept
  {
    return make<lit_expr>(tok);
  }

  id_expr* builder::make_id(const token& tok, semantics::symbol& sym) noexcept
  {
    return make<id_expr>(tok, sym);
  }

  typed_expr* builder::make_typed(const token& kw, invocation::arg_list args) noexcept
  {
    return make<typed_expr>(kw, std::move(args));
  }

  call_expr* builder::make_call(expr& callable, call_expr::arg_list args) noexcept
  {
    return make<call_expr>(callable, std::move(args));
  }

  paren_expr* builder::make_paren(expr& e, const token& op) noexcept
  {
    return make<paren_expr>(e, op);
  }

  unary_expr* builder::make_unary(expr& e, const token& op) noexcept
  {
    return make<unary_expr>(e, op);
  }

  binary_expr* builder::make_binary(expr& left, expr& right, const token& op) noexcept
  {
    return make<binary_expr>(left, right, op);
  }

  assign_expr* builder::make_assign(expr& left, expr& right, const token& op) noexcept
  {
    return make<assign_expr>(left, right, op);
  }

  pattern* builder::make_pattern(const token& op, expr* checkedExpr, scope& body) noexcept
  {
    return make<pattern>(op, checkedExpr, body);
  }

  cond_expr* builder::make_conditional(expr& condition, cond_expr::child_list children) noexcept
  {
    return make<cond_expr>(condition, std::move(children));
  }


  // Declarators

  decl_expr* builder::make_decl_expr(decl& d) noexcept
  {
    return make<decl_expr>(d);
  }

  var_decl* builder::make_var_decl(const token& var, expr& initialiser) noexcept
  {
    return make<var_decl>(var, initialiser);
  }

  param_decl* builder::make_param_decl(const token& name, expr* opt) noexcept
  {
    return make<param_decl>(name, opt);
  }

  func_decl* builder::make_func_decl(const token& func, scope& def, func_decl::param_list params) noexcept
  {
    return make<func_decl>(func, def, std::move(params));
  }

  // Private members

}