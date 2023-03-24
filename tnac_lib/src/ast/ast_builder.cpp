#include "ast/ast_builder.hpp"

namespace tnac::ast
{
  // Special members

  builder::builder() noexcept = default;
  builder::~builder() noexcept = default;

  // Public members

  scope* builder::make_scope(node* parent, scope::elem_list children) noexcept
  {
    return make<scope>(parent, std::move(children));
  }

  expr* builder::make_literal(node* parent, const token& tok) noexcept
  {
    return make<lit_expr>(parent, tok);
  }

  // Private members



}