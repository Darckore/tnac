#include "ast/ast_builder.hpp"

namespace tnac::ast
{
  // Special members

  builder::builder() noexcept = default;
  builder::~builder() noexcept = default;

  // Public members

  scope* builder::make_scope(scope::elem_list children) noexcept
  {
    return make<scope>(std::move(children));
  }

  expr* builder::make_literal(const token& tok) noexcept
  {
    return make<lit_expr>(tok);
  }

  paren_expr* builder::make_paren(expr& e) noexcept
  {
    return make<paren_expr>(e);
  }

  unary_expr* builder::make_unary(expr& e, const token& op) noexcept
  {
    return make<unary_expr>(e, op);
  }

  binary_expr* builder::make_binary(expr& left, expr& right, const token& op) noexcept
  {
    return make<binary_expr>(left, right, op);
  }

  // Private members

}