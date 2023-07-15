#include "ast/ast_expr.hpp"

namespace tnac::ast
{
  // Base expr

  expr::~expr() noexcept = default;

  expr::expr(kind k, const token& tok) noexcept :
    node{ k },
    m_pos{ tok }
  {}

  const token& expr::pos() const noexcept
  {
    return m_pos;
  }


  // Result expr

  result_expr::~result_expr() noexcept = default;

  result_expr::result_expr(const token& tok) noexcept :
    expr{ kind::Result, tok }
  {}


  // Ret expression

  ret_expr::~ret_expr() noexcept = default;

  ret_expr::ret_expr(expr& retVal, const token& kwPos) noexcept :
    expr{ kind::Ret, kwPos },
    m_retVal{ &retVal }
  {
    assume_ancestry(m_retVal);
  }

  const expr& ret_expr::returned_value() const noexcept
  {
    return *m_retVal;
  }
  expr& ret_expr::returned_value() noexcept
  {
    return FROM_CONST(returned_value);
  }


  // Literal expr

  lit_expr::~lit_expr() noexcept = default;

  lit_expr::lit_expr(const token& tok) noexcept :
    expr{ kind::Literal, tok }
  {}


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


  // Array expr

  array_expr::~array_expr() noexcept = default;

  array_expr::array_expr(const token& ob, elem_list elements) noexcept :
    expr{ kind::Array, ob },
    m_elements{ std::move(elements) }
  {
    for (auto e : m_elements)
    {
      assume_ancestry(e);
    }
  }

  const array_expr::elem_list& array_expr::elements() const noexcept
  {
    return m_elements;
  }
  array_expr::elem_list& array_expr::elements() noexcept
  {
    return FROM_CONST(elements);
  }


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


  // Abs expr

  abs_expr::~abs_expr() noexcept = default;

  abs_expr::abs_expr(expr& e, const token& op) noexcept :
    expr{ kind::Abs, op },
    m_expr{ &e }
  {
    assume_ancestry(m_expr);
  }

  const expr& abs_expr::expression() const noexcept
  {
    return *m_expr;
  }
  expr& abs_expr::expression() noexcept
  {
    return FROM_CONST(expression);
  }


  // Invocation

  invocation::~invocation() noexcept = default;

  invocation::invocation(node_kind kind, const token& name, arg_list args) noexcept :
    expr{ kind, name },
    m_args{ std::move(args) }
  {
    for (auto arg : m_args)
    {
      assume_ancestry(arg);
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

  call_expr::call_expr(expr& callable, arg_list args) noexcept :
    expr{ kind::Call, callable.pos() },
    m_args{ std::move(args) },
    m_callee{ &callable }
  {
    assume_ancestry(&callable);
    for (auto arg : m_args)
    {
      assume_ancestry(arg);
    }
  }

  const expr& call_expr::callable() const noexcept
  {
    return *m_callee;
  }
  expr& call_expr::callable() noexcept
  {
    return FROM_CONST(callable);
  }

  const call_expr::arg_list& call_expr::args() const noexcept
  {
    return m_args;
  }
  call_expr::arg_list& call_expr::args() noexcept
  {
    return FROM_CONST(args);
  }


  // Matcher

  matcher::~matcher() noexcept = default;

  matcher::matcher(const token& op, expr* checkedExpr) noexcept :
    expr{ kind::Matcher, op },
    m_checked{ checkedExpr }
  {
    assume_ancestry(checkedExpr);
  }

  bool matcher::is_default() const noexcept
  {
    return !m_checked && !is_unary();
  }

  bool matcher::is_unary() const noexcept
  {
    using enum tok_kind;
    return !m_checked && pos().is_any(Exclamation, Question);
  }

  bool matcher::has_implicit_op() const noexcept
  {
    using enum tok_kind;
    return !pos().is_any(Eq, NotEq, Less, LessEq, Greater, GreaterEq);
  }

  const expr& matcher::checked() const noexcept
  {
    UTILS_ASSERT(!is_default());
    return *m_checked;
  }
  expr& matcher::checked() noexcept
  {
    return FROM_CONST(checked);
  }


  // Pattern

  pattern::~pattern() noexcept = default;

  pattern::pattern(expr& matcherExpr, scope& body) noexcept :
    expr{ kind::Pattern, matcherExpr.pos() },
    m_matcher{ &matcherExpr },
    m_body{ &body }
  {
    assume_ancestry(&matcherExpr);
    assume_ancestry(&body);
  }

  const expr& pattern::matcher() const noexcept
  {
    return *m_matcher;
  }
  expr& pattern::matcher() noexcept
  {
    return FROM_CONST(matcher);
  }

  const scope& pattern::body() const noexcept
  {
    return *m_body;
  }
  scope& pattern::body() noexcept
  {
    return FROM_CONST(body);
  }


  // Conditional expr

  cond_expr::~cond_expr() noexcept = default;

  cond_expr::cond_expr(expr& condition, scope& body) noexcept :
    expr{ kind::Cond, condition.pos() },
    m_cond{ &condition },
    m_body{ &body }
  {
    assume_ancestry(&condition);
    assume_ancestry(&body);
  }

  const expr& cond_expr::cond() const noexcept
  {
    return *m_cond;
  }
  expr& cond_expr::cond() noexcept
  {
    return FROM_CONST(cond);
  }

  const scope& cond_expr::patterns() const noexcept
  {
    return *m_body;
  }
  scope& cond_expr::patterns() noexcept
  {
    return FROM_CONST(patterns);
  }


  // Conditional shorthand expr

  cond_short::~cond_short() noexcept = default;

  cond_short::cond_short(expr& condition, expr* onTrue, expr* onFalse, scope& sc) noexcept :
    expr{ kind::CondShort, condition.pos() },
    m_cond{ &condition },
    m_true{ onTrue },
    m_false{ onFalse }
  {
    assume_ancestry(&condition);
    assume_ancestry(onTrue);
    assume_ancestry(onFalse);
    assume_ancestry(&sc);
  }

  const expr& cond_short::cond() const noexcept
  {
    return *m_cond;
  }
  expr& cond_short::cond() noexcept
  {
    return FROM_CONST(cond);
  }

  bool cond_short::has_true() const noexcept
  {
    return static_cast<bool>(m_true);
  }
  bool cond_short::has_false() const noexcept
  {
    return static_cast<bool>(m_false);
  }

  const expr& cond_short::on_true() const noexcept
  {
    UTILS_ASSERT(has_true());
    return *m_true;
  }
  expr& cond_short::on_true() noexcept
  {
    return FROM_CONST(on_true);
  }

  const expr& cond_short::on_false() const noexcept
  {
    UTILS_ASSERT(has_false());
    return *m_false;
  }
  expr& cond_short::on_false() noexcept
  {
    return FROM_CONST(on_false);
  }

}