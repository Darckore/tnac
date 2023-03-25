#include "ast/ast_expr.hpp"

namespace tnac::ast
{
  // Base expr

  expr::~expr() noexcept = default;

  expr::expr(kind k) noexcept :
    node{ k }
  {}


  // Error expr

  error_expr::~error_expr() noexcept = default;

  error_expr::error_expr(const token& tok, string_t msg) noexcept :
    expr{ kind::Error },
    m_errMsg{ msg },
    m_pos{ tok }
  {}

  const token& error_expr::at() const noexcept
  {
    return m_pos;
  }

  string_t error_expr::message() const noexcept
  {
    return m_errMsg;
  }


  // Literal expr

  lit_expr::~lit_expr() noexcept = default;

  lit_expr::lit_expr(const token& tok) noexcept :
    expr{ kind::Literal },
    m_value{ tok }
  {
  }

  const token& lit_expr::value() const noexcept
  {
    return m_value;
  }


  // Id expr

  id_expr::~id_expr() noexcept = default;

  id_expr::id_expr(const token& tok) noexcept :
    expr{ kind::Identifier },
    m_id{ tok }
  {}

  string_t id_expr::name() const noexcept
  {
    return pos().m_value;
  }

  const token& id_expr::pos() const noexcept
  {
    return m_id;
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

  const token& unary_expr::op() const noexcept
  {
    return m_op;
  }

  const expr& unary_expr::operand() const noexcept
  {
    return *m_expr;
  }
  expr& unary_expr::operand() noexcept
  {
    return utils::mutate(std::as_const(*this).operand());
  }


  // Binary expr

  binary_expr::~binary_expr() noexcept = default;

  binary_expr::binary_expr(expr& left, expr& right, const token& op) noexcept :
    expr{ kind::Binary },
    m_left{ &left },
    m_right{ &right },
    m_op{ op }
  {
    assume_ancestry(m_left);
    assume_ancestry(m_right);
  }

  const token& binary_expr::op() const noexcept
  {
    return m_op;
  }

  const expr& binary_expr::left() const noexcept
  {
    return *m_left;
  }
  expr& binary_expr::left() noexcept
  {
    return utils::mutate(std::as_const(*this).left());
  }

  const expr& binary_expr::right() const noexcept
  {
    return *m_right;
  }
  expr& binary_expr::right() noexcept
  {
    return utils::mutate(std::as_const(*this).right());
  }


  // Paren expr

  paren_expr::~paren_expr() noexcept = default;

  paren_expr::paren_expr(expr& e) noexcept :
    expr{ kind::Paren },
    m_expr{ &e }
  {
    assume_ancestry(m_expr);
  }

  const expr& paren_expr::internal_expr() const noexcept
  {
    return *m_expr;
  }
  expr& paren_expr::internal_expr() noexcept
  {
    return utils::mutate(std::as_const(*this).internal_expr());
  }
}