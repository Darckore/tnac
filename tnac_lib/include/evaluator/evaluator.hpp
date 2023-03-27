//
// Expression evaluator
//

#pragma once
#include "ast/ast_visitor.hpp"
#include "evaluator/value_visitor.hpp"
#include "evaluator/value_registry.hpp"

namespace tnac
{
  //
  // Evaluator for expressions
  //
  class evaluator : public ast::bottom_up_visitor<evaluator>
  {
  public:
    CLASS_SPECIALS_NONE(evaluator);

    explicit evaluator(eval::registry& registry) noexcept;

  public:
    void visit(ast::binary_expr* binary) noexcept;

    void visit(ast::unary_expr* unary) noexcept;

    void visit(ast::paren_expr* paren) noexcept;

    void visit(ast::lit_expr* lit) noexcept;

  private:
    //
    // Adds a value to the registry
    //
    template <eval::detail::expr_result T>
    eval::value reg(T val) noexcept
    {
      if constexpr (is_same_noquals_v<T, int_type>)
        return m_registry.register_int(val);
      else if constexpr (is_same_noquals_v<T, float_type>)
        return m_registry.register_float(val);
      else
        return {};
    }

    //
    // Generalised cast
    //
    template <eval::detail::expr_result From, eval::detail::expr_result To>
    eval::value cast(eval::value val) noexcept
    {
      auto res = static_cast<To>(val.get<From>());
      return reg(res);
    }

    //
    // Generalised binary
    //
    template <eval::detail::expr_result T, typename F>
      requires (std::is_invocable_r_v<T, F, T, T>)
    eval::value binary_op(eval::value lhs, eval::value rhs, F&& op) noexcept
    {
      auto res = op(lhs.get<T>(), rhs.get<T>());
      return reg(res);
    }

    //
    // Generalised binary
    //
    template <typename F>
    eval::value binary_op(eval::value lhs, eval::value rhs, F&& op) noexcept
    {
      using enum eval::type_id;
      const auto ti = common_type(lhs, rhs);
      lhs = cast(lhs, ti);
      rhs = cast(rhs, ti);
      switch (ti)
      {
      case Int:
        return binary_op<int_type>(lhs, rhs, std::forward<F>(op));

      case Float:
        return binary_op<float_type>(lhs, rhs, std::forward<F>(op));

      default:
        return {};
      }
    }

    //
    // Casts a value to the given type
    //
    eval::value cast(eval::value val, eval::type_id dest) noexcept;

    //
    // Calculates common type according to rank
    // The rank, highest to lowest, is:
    //  Float
    //  Int
    //
    eval::type_id common_type(eval::value v1, eval::value v2) noexcept;

    //
    // Adds two values
    //
    eval::value add(eval::value lhs, eval::value rhs) noexcept;

    //
    // Subtracts two values
    //
    eval::value sub(eval::value lhs, eval::value rhs) noexcept;

    //
    // Multiplies two values
    //
    eval::value mul(eval::value lhs, eval::value rhs) noexcept;

    //
    // Divides two values
    //
    eval::value div(eval::value lhs, eval::value rhs) noexcept;

    //
    // Evaluates a binary expression
    //
    eval::value eval_binary(eval::value lhs, eval::value rhs, token::kind op) noexcept;

    //
    // Evaluates a literal and returns its value
    //
    eval::value eval_token(const token& tok) noexcept;

  private:
    eval::registry& m_registry;
    eval::value_visitor m_visitor;
  };
}