#include "evaluator/evaluator.hpp"
#include "evaluator/value.hpp"

namespace tnac
{
  namespace detail
  {
    namespace
    {
      constexpr auto conv_unary(tok_kind tk) noexcept
      {
        return tk == tok_kind::Minus ?
          eval::val_ops::UnaryNegation :
          eval::val_ops::UnaryPlus;
      }
    }
  }

  // Special members

  evaluator::evaluator(eval::registry& registry) noexcept :
    m_registry{ registry },
    m_visitor{ registry }
  {}


  // Public members

  void evaluator::visit(ast::binary_expr* binary) noexcept
  {
    auto&& left = binary->left();
    auto&& right = binary->right();
    const auto op = binary->op().m_kind;
    auto res = eval_binary(left.value(), right.value(), op);
    binary->eval_result(res);
  }

  void evaluator::visit(ast::unary_expr* unary) noexcept
  {
    const auto opCode = detail::conv_unary(unary->op().m_kind);
    auto val = unary->operand().value();
    unary->eval_result(m_visitor.visit_unary(val, opCode));
  }

  void evaluator::visit(ast::paren_expr* paren) noexcept
  {
    paren->eval_result(paren->internal_expr().value());
  }

  void evaluator::visit(ast::lit_expr* lit) noexcept
  {
    auto value = eval_token(lit->pos());
    lit->eval_result(value);
  }


  // Private members

  eval::type_id evaluator::common_type(eval::value v1, eval::value v2) noexcept
  {
    using enum eval::type_id;
    auto resType = v1.id();
    if (v2.id() == Float)
      resType = Float;

    return resType;
  }

  eval::value evaluator::cast(eval::value val, eval::type_id dest) noexcept
  {
    using enum eval::type_id;
    const auto ti = val.id();
    if (ti == dest)
      return val;

    if (ti == Int)
    {
      if (dest == Float)
        return cast<int_type, float_type>(val);
    }
    else if (ti == Float)
    {
      if (dest == Int)
        return cast<float_type, int_type>(val);
    }

    return val;
  }

  eval::value evaluator::add(eval::value lhs, eval::value rhs) noexcept
  {
    return binary_op(lhs, rhs, [](auto l, auto r)
      {
        return l + r;
      });
  }

  eval::value evaluator::sub(eval::value lhs, eval::value rhs) noexcept
  {
    return binary_op(lhs, rhs, [](auto l, auto r)
      {
        return l - r;
      });
  }

  eval::value evaluator::mul(eval::value lhs, eval::value rhs) noexcept
  {
    return binary_op(lhs, rhs, [](auto l, auto r)
      {
        return l * r;
      });
  }

  eval::value evaluator::div(eval::value lhs, eval::value rhs) noexcept
  {
    return binary_op(lhs, rhs, [](auto l, auto r)
      {
        return l / r;
      });
  }

  eval::value evaluator::eval_binary(eval::value lhs, eval::value rhs, token::kind op) noexcept
  {
    if (!lhs || !rhs)
      return {};

    using enum tok_kind;
    switch (op)
    {
    case Plus:
      return add(lhs, rhs);

    case Minus:
      return sub(lhs, rhs);

    case Asterisk:
      return mul(lhs, rhs);

    case Slash:
      return div(lhs, rhs);

    default:
      return {};
    }
  }

  eval::value evaluator::eval_token(const token& tok) noexcept
  {
    switch (tok.m_kind)
    {
    case token::IntDec:
      return m_visitor.visit_int_literal(tok.m_value, 10);
    case token::IntBin:
      return m_visitor.visit_int_literal(tok.m_value, 2);
    case token::IntOct:
      return m_visitor.visit_int_literal(tok.m_value, 8);
    case token::IntHex:
      return m_visitor.visit_int_literal(tok.m_value, 16);

    case token::Float:
      return m_visitor.visit_float_literal(tok.m_value);

    default:
      return {};
    }
  }
}