#include "ast/ast_expr.hpp"

namespace tnac::ast
{
  // Base expr

  expr::~expr() noexcept = default;

  expr::expr(kind k, const token& tok) noexcept :
    node{ k },
    m_pos{ tok }
  {}

  eval::value expr::value() const noexcept
  {
    return m_val;
  }

  void expr::eval_result(eval::value val) noexcept
  {
    m_val = val;
  }

  const token& expr::pos() const noexcept
  {
    return m_pos;
  }


  // Result expr

  result_expr::~result_expr() noexcept = default;

  result_expr::result_expr(const token& tok) noexcept :
    expr{ kind::Result, tok }
  {}


  // Literal expr

  lit_expr::~lit_expr() noexcept = default;

  lit_expr::lit_expr(const token& tok) noexcept :
    expr{ kind::Literal, tok }
  {
  }


  // Id expr

  id_expr::~id_expr() noexcept = default;

  id_expr::id_expr(const token& tok, semantics::symbol& sym) noexcept :
    expr{ kind::Identifier, tok },
    m_sym{ &sym }
  {}

  string_t id_expr::name() const noexcept
  {
    return pos().m_value;
  }

  const semantics::symbol& id_expr::symbol() const noexcept
  {
    return *m_sym;
  }
  semantics::symbol& id_expr::symbol() noexcept
  {
    return FROM_CONST(symbol);
  }


  // Unary expr

  unary_expr::~unary_expr() noexcept = default;

  unary_expr::unary_expr(expr& e, const token& op) noexcept :
    expr{ kind::Unary, op },
    m_expr{ &e }
  {
    assume_ancestry(m_expr);
  }

  const token& unary_expr::op() const noexcept
  {
    return pos();
  }

  const expr& unary_expr::operand() const noexcept
  {
    return *m_expr;
  }
  expr& unary_expr::operand() noexcept
  {
    return FROM_CONST(operand);
  }


  // Binary expr

  binary_expr::~binary_expr() noexcept = default;

  binary_expr::binary_expr(kind k, expr& left, expr& right, const token& op) noexcept :
    expr{ k, left.pos() },
    m_left{ &left },
    m_right{ &right },
    m_op{ op }
  {
    assume_ancestry(m_left);
    assume_ancestry(m_right);
  }

  binary_expr::binary_expr(expr& left, expr& right, const token& op) noexcept :
    binary_expr{ kind::Binary, left, right, op }
  {
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
    return FROM_CONST(left);
  }

  const expr& binary_expr::right() const noexcept
  {
    return *m_right;
  }
  expr& binary_expr::right() noexcept
  {
    return FROM_CONST(right);
  }


  // Assign expression

  assign_expr::~assign_expr() noexcept = default;

  assign_expr::assign_expr(expr& assignee, expr& assigned, const token& op) noexcept :
    binary_expr{ kind::Assign, assignee, assigned, op }
  {}


  // Paren expr

  paren_expr::~paren_expr() noexcept = default;

  paren_expr::paren_expr(expr& e, const token& op) noexcept :
    expr{ kind::Paren, op },
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
    return FROM_CONST(internal_expr);
  }


  // Invocation

  invocation::~invocation() noexcept = default;

  invocation::invocation(node_kind kind, const token& name, arg_list args) noexcept :
    expr{ kind, name },
    m_args{ std::move(args) }
  {
    for (auto par : m_args)
    {
      assume_ancestry(par);
    }
  }

  const token& invocation::name() const noexcept
  {
    return pos();
  }

  const invocation::arg_list& invocation::args() const noexcept
  {
    return m_args;
  }
  invocation::arg_list& invocation::args() noexcept
  {
    return FROM_CONST(args);
  }


  // Typed expr

  typed_expr::~typed_expr() noexcept = default;

  typed_expr::typed_expr(const token& typeName, arg_list args) noexcept :
    invocation{ kind::Typed, typeName, std::move(args) }
  {
  }

  const token& typed_expr::type_name() const noexcept
  {
    return name();
  }


  // Call expr

  call_expr::~call_expr() noexcept = default;

  call_expr::call_expr(const token& callable, arg_list args, semantics::symbol& sym) noexcept :
    invocation{ kind::Call, callable, std::move(args) },
    m_sym{ &sym }
  {}

  const token& call_expr::callable_name() const noexcept
  {
    return name();
  }

  const semantics::symbol& call_expr::symbol() const noexcept
  {
    return *m_sym;
  }
  semantics::symbol& call_expr::symbol() noexcept
  {
    return FROM_CONST(symbol);
  }

}