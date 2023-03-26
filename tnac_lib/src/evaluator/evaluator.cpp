#include "evaluator/evaluator.hpp"
#include "evaluator/value.hpp"

namespace tnac
{
  // Special members

  evaluator::evaluator(eval::registry& registry) noexcept :
    m_registry{ registry }
  {}


  // Public members

  void evaluator::visit(ast::unary_expr* unary) noexcept
  {
    utils::unused(unary);
  }

  void evaluator::visit(ast::lit_expr* lit) noexcept
  {
    auto value = eval_token(lit->pos());
    lit->eval_result(value);
  }


  // Private members

  eval::value evaluator::eval_token(const token& tok) noexcept
  {
    switch (tok.m_kind)
    {
    case token::IntDec:
    case token::IntBin:
    case token::IntOct:
    case token::IntHex:
      return eval_int(tok.m_value);

    default:
      return {};
    }
  }

  eval::value evaluator::eval_int(string_t src) noexcept
  {
    auto begin = src.data();
    auto end = begin + src.length();

    int_type result{};
    auto convRes = std::from_chars(begin, end, result);
    if (convRes.ec != std::errc{ 0 })
      return {};

    return m_registry.register_int(result);
  }

}