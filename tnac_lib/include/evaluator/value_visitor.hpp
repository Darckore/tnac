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
    concept unary_func = std::is_invocable_v<F, T>;

    template <typename F, typename T1, typename T2>
    concept binary_func = std::is_invocable_v<F, T1, T2>;
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
    template <detail::expr_result T>
    value reg_value(T) noexcept;

    template <>
    value reg_value<int_type>(int_type val) noexcept
    {
      return m_registry.register_int(val);
    }

    template <>
    value reg_value<float_type>(float_type val) noexcept
    {
      return m_registry.register_float(val);
    }

    template <detail::expr_result T>
    value visit_unary(T val, val_ops op) noexcept
    {
      auto result = (op == UnaryNegation) ? -val : +val;
      return reg_value<T>(result);
    }

    //
    // Extracts type from value and calls the specified function
    //
    template <typename F>
    value visit_value(value val, F&& func) noexcept
    {
      using enum type_id;
      switch (val.id())
      {
      case Int:
        return func(val.get<int_type>());

      case Float:
        return func(val.get<float_type>());

      default:
        return val;
      }
    }

  public:
    //
    // Returns a resulting value from a unary expr
    //
    value visit_unary(value val, val_ops op) noexcept
    {
      if (!val || utils::eq_none(op, UnaryNegation, UnaryPlus))
        return val;

      return visit_value(val, [this, op](auto v)
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

      return m_registry.register_int(result);
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

      return m_registry.register_float(result);
    }

  private:
    registry& m_registry;
  };
}