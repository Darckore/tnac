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

  lit_expr* builder::make_literal(const token& tok) noexcept
  {
    return make<lit_expr>(tok);
  }

  id_expr* builder::make_id(const token& tok) noexcept
  {
    return make<id_expr>(tok);
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

  // Declarators

  var_decl* builder::make_var_decl(id_expr& var, expr& initialiser) noexcept
  {
    return make<var_decl>(var, initialiser);
  }

  // Private members

}