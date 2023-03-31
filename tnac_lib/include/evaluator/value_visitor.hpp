//
// Value visitor
//

#pragma once
#include "evaluator/value_registry.hpp"

namespace tnac::eval
{
  //
  // Supported operations
  //
  enum class val_ops : std::uint8_t
  {
    InvalidOp,
    Addition,
    Subtraction,
    Multiplication,
    Division,
    UnaryNegation,
    UnaryPlus
  };

  namespace detail
  {
    template <typename F, typename T>
    concept unary_function = std::is_nothrow_invocable_v<F, T>;

    template <typename F, typename T1, typename T2>
    concept binary_function = std::is_nothrow_invocable_v<F, T1, T2>;

    constexpr auto is_unary(val_ops op) noexcept
    {
      using enum val_ops;
      return utils::eq_any(op, UnaryPlus, UnaryNegation);
    }
    constexpr auto is_binary(val_ops op) noexcept
    {
      using enum val_ops;
      return utils::eq_any(op, Addition, Subtraction, Multiplication, Division);
    }
  }

  //
  // Value visitor used in expression evaluations
  //
  class value_visitor
  {
  public:
    using enum val_ops;

  public:
    CLASS_SPECIALS_NONE(value_visitor);

    explicit value_visitor(registry& reg) noexcept :
      m_registry{ reg }
    {}

  private:
    //
    // Extracts type from value and calls the specified function
    //
    template <typename F>
    value visit_value(value val, F&& func) noexcept
    {
      return on_value(val, std::forward<F>(func));
    }


    template <detail::expr_result T>
    value reg_literal(T val) noexcept
    {
      return m_registry.register_literal(val);
    }

    value visit_unary(invalid_val_t, val_ops) noexcept
    {
      return {};
    }

    template <detail::expr_result T, detail::unary_function<T> F>
    value visit_unary(T val, F&& op) noexcept
    {
      return reg_literal(op(val));
    }

    template <detail::expr_result T>
    value visit_unary(T val, val_ops op) noexcept
    {
      using enum val_ops;
      switch (op)
      {
      case UnaryNegation:
        return visit_unary(val, [](auto v) noexcept { return -v; });

      case UnaryPlus:
        return visit_unary(val, [](auto v) noexcept { return +v; });

      default:
        return {};
      }
    }


    value visit_binary(invalid_val_t, invalid_val_t, val_ops) noexcept
    {
      return {};
    }
    template <typename T>
    value visit_binary(invalid_val_t, T, val_ops) noexcept
    {
      return visit_binary(invalid_val_t{}, invalid_val_t{}, val_ops{});
    }
    template <typename T>
    value visit_binary(T, invalid_val_t, val_ops) noexcept
    {
      return visit_binary(invalid_val_t{}, invalid_val_t{}, val_ops{});
    }

    template <detail::expr_result L, detail::expr_result R, detail::binary_function<L, R> F>
    value visit_binary(L lhs, R rhs, F&& op) noexcept
    {
      return reg_literal(op(lhs, rhs));
    }

    template <detail::expr_result L, detail::expr_result R>
    value visit_binary(L lhs, R rhs, val_ops op) noexcept
    {
      using enum val_ops;
      switch (op)
      {
      case Addition:
        return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l + r; });

      case Subtraction:
        return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l - r; });

      case Multiplication:
        return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l * r; });

      case Division:
        // Corner case, we don't want integral division, so, let's convert lhs to float
        if constexpr (is_same_noquals_v<decltype(lhs), int_type>)
          return visit_binary(static_cast<float_type>(lhs), rhs, [](auto l, auto r) noexcept 
            { return l / r; });
        else
          return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l / r; });

      default:
        return {};
      }
    }

    template <detail::expr_result L>
    value visit_binary(L lhs, value rhs, val_ops op) noexcept
    {
      return visit_value(rhs, [this, lhs, op](auto rhs) noexcept
        {
          return visit_binary(lhs, rhs, op);
        });
    }

  public:
    //
    // Returns a resulting value from a binary expr
    //
    value visit_binary(value lhs, value rhs, val_ops op) noexcept
    {
      if (!lhs || !rhs || !detail::is_binary(op))
        return {};

      return visit_value(lhs, [this, rhs, op](auto lhs) noexcept
        {
          return visit_binary(lhs, rhs, op);
        });
    }

    //
    // Returns a resulting value from a unary expr
    //
    value visit_unary(value val, val_ops op) noexcept
    {
      if (!val || !detail::is_unary(op))
        return {};

      return visit_value(val, [this, op](auto v) noexcept
        {
          return visit_unary(v, op);
        });
    }

    //
    // Parses an integer literal value from string
    //
    value visit_int_literal(string_t src, int base) noexcept
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

      return reg_literal(result);
    }

    //
    // Parses a floating point literal value from string
    //
    value visit_float_literal(string_t src) noexcept
    {
      auto begin = src.data();
      auto end = begin + src.length();

      float_type result{};
      auto convRes = std::from_chars(begin, end, result);
      if (convRes.ec != std::errc{ 0 })
        return {};

      return reg_literal(result);
    }

  private:
    registry& m_registry;
  };
}