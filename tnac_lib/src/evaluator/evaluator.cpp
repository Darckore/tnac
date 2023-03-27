#include "evaluator/evaluator.hpp"
#include "evaluator/value.hpp"

namespace tnac
{
  // Special members

  evaluator::evaluator(eval::registry& registry) noexcept :
    m_registry{ registry }
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
    auto&& operand = unary->operand();
    auto val = operand.value();
    if (unary->op().is(token::Minus))
    {
      val = negate(val);
    }
    unary->eval_result(val);
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

  eval::value evaluator::negate(eval::value val) noexcept
  {
    if (!val)
      return val;

    using enum eval::type_id;

    switch (val.id())
    {
    case Int:
      return m_registry.register_int(-val.get<int_type>());

    case Float:
      return m_registry.register_float(-val.get<float_type>());

    default:
      return {};
    }
  }

  eval::value evaluator::eval_token(const token& tok) noexcept
  {
    switch (tok.m_kind)
    {
    case token::IntDec:
      return eval_int(tok.m_value, 10);
    case token::IntBin:
      return eval_int(tok.m_value, 2);
    case token::IntOct:
      return eval_int(tok.m_value, 8);
    case token::IntHex:
      return eval_int(tok.m_value, 16);

    case token::Float:
      return eval_float(tok.m_value);

    default:
      return {};
    }
  }

  eval::value evaluator::eval_int(string_t src, int base) noexcept
  {
    auto prefix = string_t::size_type{};
    if (utils::eq_any(base, 2, 16))
      prefix = 2u;
    else if (base == 8)
      prefix = 1u;

    auto begin = src.data() + prefix;
    auto end = begin + src.length();

    int_type result{};
    auto convRes = std::from_chars(begin, end, result, base);
    if (convRes.ec != std::errc{ 0 })
      return {};

    return m_registry.register_int(result);
  }

  eval::value evaluator::eval_float(string_t src) noexcept
  {
    auto begin = src.data();
    auto end = begin + src.length();

    float_type result{};
    auto convRes = std::from_chars(begin, end, result);
    if (convRes.ec != std::errc{ 0 })
      return {};

    return m_registry.register_float(result);
  }

}