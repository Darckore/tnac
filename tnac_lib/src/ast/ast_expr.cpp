#include "ast/ast_expr.hpp"

namespace tnac::ast
{
  // Base expr

  expr::~expr() noexcept = default;

  expr::expr(kind k) noexcept :
    node{ k }
  {}


  // Literal expr

  lit_expr::~lit_expr() noexcept = default;

  lit_expr::lit_expr(const token& tok) noexcept :
    expr{ kind::Literal },
    m_value{ tok }
  {
  }


  // Unary expr

  unary_expr::~unary_expr() noexcept = default;

  unary_expr::unary_expr(expr& e, const token& op) noexcept :
    expr{ kind::Unary },
    m_expr{ &e },
    m_op{ op }
  {
    assume_ancestry(m_expr);
  }
}