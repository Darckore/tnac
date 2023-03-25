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

  unary_expr* builder::make_unary(expr& e, const token& op) noexcept
  {
    return make<unary_expr>(e, op);
  }

  // Private members

}