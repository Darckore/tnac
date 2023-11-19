#include "parser/ast/ast_builder.hpp"

namespace tnac::ast
{
  // Special members

  builder::builder() noexcept = default;
  builder::~builder() noexcept = default;


  // Public members(General)

  root* builder::make_root() noexcept
  {
    return make<root>();
  }

  module_def* builder::make_module(buf_t name, src::loc_wrapper loc) noexcept
  {
    return make<module_def>(std::move(name), loc);
  }

  module_def* builder::get_default_module(src::loc_wrapper loc) noexcept
  {
    static module_def fake{ {}, loc };
    fake.override_loc(loc);
    return &fake;
  }

  import_dir* builder::make_import(const token& pos, import_dir::elem_list name, import_dir::pointer alias) noexcept
  {
    return make<import_dir>(pos, std::move(name), alias);
  }

  scope* builder::make_scope(scope::elem_list children) noexcept
  {
    return make<scope>(std::move(children));
  }

  error_expr* builder::make_error(const token& pos, string_t msg) noexcept
  {
    return make<error_expr>(pos, msg);
  }


  // Public members(Expressions)

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

  array_expr* builder::make_array(const token& ob, array_expr::elem_list elements) noexcept
  {
    return make<array_expr>(ob, std::move(elements));
  }

  paren_expr* builder::make_paren(expr& e, const token& op) noexcept
  {
    return make<paren_expr>(e, op);
  }

  abs_expr* builder::make_abs(expr& e, const token& op) noexcept
  {
    return make<abs_expr>(e, op);
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

  matcher* builder::make_matcher(const token& op, expr* checkedExpr) noexcept
  {
    return make<matcher>(op, checkedExpr);
  }

  pattern* builder::make_pattern(expr& matcherExpr, scope& body) noexcept
  {
    return make<pattern>(matcherExpr, body);
  }

  cond_expr* builder::make_conditional(expr& condition, scope& body) noexcept
  {
    return make<cond_expr>(condition, body);
  }

  cond_short* builder::make_short_cond(expr& condition, expr* onTrue, expr* onFalse, scope& sc) noexcept
  {
    return make<cond_short>(condition, onTrue, onFalse, sc);
  }

  dot_expr* builder::make_dot(expr& accessed, expr& accessor) noexcept
  {
    return make<dot_expr>(accessed, accessor);
  }


  // Public members(Declarators)

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

  func_decl* builder::make_func_decl(const token& func, const token& pos, scope& def, func_decl::param_list params) noexcept
  {
    return make<func_decl>(func, pos, def, std::move(params));
  }
}