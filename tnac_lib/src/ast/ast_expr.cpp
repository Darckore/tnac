#include "ast/ast_expr.hpp"

namespace tnac::ast
{
  // Base expr

  expr::~expr() noexcept = default;

  expr::expr(node* parent, kind k) noexcept :
    node{ parent, k }
  {}


  //
  // Literal expr
  //

  lit_expr::~lit_expr() noexcept = default;

  lit_expr::lit_expr(node* parent, const token& tok) noexcept :
    expr{ parent, kind::Literal },
    m_value{ tok }
  {
  }
}