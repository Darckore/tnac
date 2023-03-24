#include "ast/ast_expr.hpp"

namespace tnac::ast
{
  // Base expr

  expr::~expr() noexcept = default;

  expr::expr(node* parent, kind k) noexcept :
    node{ parent, k }
  {}
}