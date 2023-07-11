//
// Type traits
//

#pragma once
#include "evaluator/value/value.hpp"

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
    Modulo,
    RelLess,
    RelLessEq,
    RelGr,
    RelGrEq,
    Equal,
    NEqual,
    BitwiseAnd,
    BitwiseXor,
    BitwiseOr,
    BinaryPow,
    BinaryRoot,
    UnaryNegation,
    UnaryPlus,
    UnaryBitwiseNot,
    LogicalNot,
    LogicalIs,
    AbsoluteValue
  };


  //
  // Type info
  //

  namespace detail
  {
    template <std::size_t MinArgs, type_id... Args>
    struct basic_type_info
    {
      static constexpr std::array params{ Args... };
      static constexpr auto minArgs = MinArgs;
      static constexpr auto maxArgs = sizeof...(Args);
    };
  }

  template <typename T> struct type_info;
  template <> struct type_info<bool_type>     : detail::basic_type_info<0, type_id::Bool> {};
  template <> struct type_info<int_type>      : detail::basic_type_info<0, type_id::Int> {};
  template <> struct type_info<float_type>    : detail::basic_type_info<0, type_id::Float> {};
  template <> struct type_info<complex_type>  : detail::basic_type_info<0, type_id::Float, type_id::Float> {};
  template <> struct type_info<fraction_type> : detail::basic_type_info<2, type_id::Int, type_id::Int> {};
  template <> struct type_info<function_type> : detail::basic_type_info<1, type_id::Function> {};


  //
  // Value casters
  //

  namespace detail
  {
    template <typename T>
    concept generic_type = expr_result<T> || is_same_noquals_v<T, invalid_val_t>;
  }

  template <detail::generic_type T> using typed_value = std::optional<T>;

  template <detail::expr_result T> struct type_wrapper
  {
    using value_type  = T;
    using result_type = typed_value<value_type>;

    template <typename ...Args>
    result_type operator()(typed_value<Args>&& ...args ) noexcept
    {
      if (utils::eq_any(false, static_cast<bool>(args)...))
      {
        return {};
      }

      return value_type{ (*args)... };
    }
  };

  namespace detail
  {
    inline auto to_int(float_type val) noexcept -> typed_value<int_type>
    {
      const auto conv = static_cast<int_type>(val);
      if (utils::eq(static_cast<float_type>(conv), val))
        return conv;

      return {};
    }
    inline auto to_int(complex_type val) noexcept -> typed_value<int_type>
    {
      if (!utils::eq(val.imag(), float_type{}))
        return {};

      return to_int(val.real());
    }
  }

  template <typename To> auto get_caster() noexcept;
  template <> inline auto get_caster<bool_type>() noexcept
  {
    using res_type = typed_value<bool_type>;
    return utils::visitor
    {
      [](bool_type v) noexcept  -> res_type { return v; },
      [](int_type v) noexcept   -> res_type { return static_cast<bool>(v); },
      [](float_type v) noexcept -> res_type { return !utils::eq(v, float_type{}); },
      [](complex_type v) noexcept -> res_type
      {
        constexpr auto cmp = float_type{};
        return !utils::eq(v.real(), cmp) || !utils::eq(v.imag(), cmp);
      },
      [](fraction_type v) noexcept -> res_type { return static_cast<bool>(v.num()); },
      [](function_type) noexcept   -> res_type { return true; },
      [](array_type) noexcept      -> res_type { return true; },
      [](invalid_val_t) noexcept   -> res_type { return false; }
    };
  }
  template <> inline auto get_caster<int_type>() noexcept
  {
    using res_type = typed_value<int_type>;
    return utils::visitor
    {
      [](bool_type v) noexcept     -> res_type { return static_cast<int_type>(v); },
      [](int_type v) noexcept      -> res_type { return v; },
      [](float_type v) noexcept    -> res_type { return detail::to_int(v); },
      [](complex_type v) noexcept  -> res_type { return detail::to_int(v); },
      [](fraction_type v) noexcept -> res_type { return detail::to_int(v.to<float_type>()); },
      [](function_type) noexcept   -> res_type { return {}; },
      [](array_type) noexcept      -> res_type { return {}; },
      [](invalid_val_t) noexcept   -> res_type { return int_type{}; }
    };
  }
  template <> inline auto get_caster<float_type>() noexcept
  {
    using res_type = typed_value<float_type>;
    return utils::visitor
    {
      [](bool_type v) noexcept  -> res_type { return v ? float_type{ 1.0 } : float_type{}; },
      [](int_type v) noexcept   -> res_type { return static_cast<float_type>(v); },
      [](float_type v) noexcept -> res_type { return v; },
      [](complex_type v) noexcept -> res_type
      {
        if (!utils::eq(v.imag(), float_type{}))
          return {};

        return v.real();
      },
      [](fraction_type v) noexcept -> res_type { return v.to<float_type>(); },
      [](function_type) noexcept   -> res_type { return {}; },
      [](array_type) noexcept      -> res_type { return {}; },
      [](invalid_val_t) noexcept   -> res_type { return float_type{}; }
    };
  }
  template <> inline auto get_caster<complex_type>() noexcept
  {
    using res_type = typed_value<complex_type>;
    return utils::visitor
    {
      [](bool_type v) noexcept     -> res_type { return v ? complex_type{ 1.0, 0.0 } : complex_type{}; },
      [](int_type v) noexcept      -> res_type { return complex_type{ static_cast<float_type>(v) }; },
      [](float_type v) noexcept    -> res_type { return complex_type{ v }; },
      [](complex_type v) noexcept  -> res_type { return v; },
      [](fraction_type v) noexcept -> res_type { return complex_type{ v.to<float_type>() }; },
      [](function_type) noexcept   -> res_type { return {}; },
      [](array_type) noexcept      -> res_type { return {}; },
      [](invalid_val_t) noexcept   -> res_type { return complex_type{}; }
    };
  }
  template <> inline auto get_caster<fraction_type>() noexcept
  {
    using res_type = typed_value<fraction_type>;
    return utils::visitor
    {
      [](bool_type v) noexcept  -> res_type { return v ? fraction_type{ 1, 1 } : fraction_type{ 0, 1 }; },
      [](int_type v) noexcept   -> res_type { return fraction_type{ v }; },
      [](float_type v) noexcept -> res_type
      {
        // todo: float to frac
        if (std::isnan(v) || std::isinf(v))
        {
          const auto sign = static_cast<fraction_type::sign_t>(utils::sign(v));
          return fraction_type{ int_type{ 1 }, int_type{ }, sign };
        }

        return fraction_type{ static_cast<int_type>(v) };
      },
      [](complex_type v) noexcept -> res_type
      {
        // todo: float to frac
        if (!utils::eq(v.imag(), float_type{}))
          return {};

        if (const auto real = v.real(); std::isnan(real) || std::isinf(real))
        {
          const auto sign = static_cast<fraction_type::sign_t>(utils::sign(real));
          return fraction_type{ int_type{ 1 }, int_type{ }, sign };
        }

        return fraction_type{ static_cast<int_type>(v.real()) };
      },
      [](fraction_type v) noexcept -> res_type { return v; },
      [](function_type) noexcept   -> res_type { return {}; },
      [](array_type) noexcept      -> res_type { return {}; },
      [](invalid_val_t) noexcept   -> res_type { return fraction_type{ 0 }; }
    };
  }
  template <> inline auto get_caster<function_type>() noexcept
  {
    using res_type = typed_value<function_type>;
    return utils::visitor
    {
      [](auto) noexcept            -> res_type { return {}; },
      [](function_type v) noexcept -> res_type { return v; }
    };
  }
  template <> inline auto get_caster<array_type>() noexcept
  {
    using res_type = typed_value<array_type>;
    return utils::visitor
    {
      [](auto) noexcept         -> res_type { return {}; },
      [](array_type v) noexcept -> res_type { return v; }
    };
  }
  template <> inline auto get_caster<invalid_val_t>() noexcept
  {
    using res_type = typed_value<invalid_val_t>;
    return utils::visitor
    {
      [](auto) noexcept -> res_type { return invalid_val_t{}; }
    };
  }

  template <detail::expr_result T>
  auto cast_value(value val) noexcept { return on_value(val, get_caster<T>()); };


  //
  // Common type
  //

  template <detail::generic_type T1, detail::generic_type T2> struct common_type;
  template <detail::generic_type T1, detail::generic_type T2> using common_type_t = typename common_type<T1, T2>::type;

  // common type with self

  template <detail::generic_type T1, detail::generic_type T2> requires (is_same_noquals_v<T1, T2>)
  struct common_type<T1, T2> { using type = std::remove_cvref_t<T1>; };

  // invalid

  template <detail::generic_type T> requires (!is_same_noquals_v<T, invalid_val_t>)
  struct common_type<invalid_val_t, T> { using type = invalid_val_t; };
  template <detail::generic_type T> requires (!is_same_noquals_v<T, invalid_val_t>)
  struct common_type<T, invalid_val_t> : common_type<invalid_val_t, T> {};

  // array

  template <detail::expr_result T> requires (!is_same_noquals_v<T, array_type>)
  struct common_type<array_type, T> { using type = array_type; };
  template <detail::expr_result T> requires (!is_same_noquals_v<T, array_type>)
  struct common_type<T, array_type> : common_type<array_type, T> {};

  // function

  template <detail::expr_result T> requires (!is_same_noquals_v<T, function_type>)
  struct common_type<function_type, T> { using type = function_type; };
  template <detail::expr_result T> requires (!is_same_noquals_v<T, function_type>)
  struct common_type<T, function_type> : common_type<function_type, T> {};
  template <> struct common_type<function_type, array_type> { using type = array_type; };
  template <> struct common_type<array_type, function_type> : common_type<function_type, array_type> {};

  // complex

  template <> struct common_type<complex_type, float_type> { using type = complex_type; };
  template <> struct common_type<complex_type, fraction_type> : common_type<complex_type, float_type> {};
  template <> struct common_type<complex_type, int_type> : common_type<complex_type, float_type> {};
  template <> struct common_type<complex_type, bool_type> : common_type<complex_type, float_type> {};

  // float

  template <>
  struct common_type<float_type, complex_type> : common_type<complex_type, float_type> {};
  template <> struct common_type<float_type, fraction_type> { using type = float_type; };
  template <> struct common_type<float_type, int_type> : common_type<float_type, fraction_type> {};
  template <> struct common_type<float_type, bool_type> : common_type<float_type, fraction_type> {};

  // fraction

  template <> struct common_type<fraction_type, complex_type> : common_type<complex_type, fraction_type> {};
  template <> struct common_type<fraction_type, float_type> : common_type<float_type, fraction_type> {};
  template <> struct common_type<fraction_type, int_type> { using type = fraction_type; };
  template <> struct common_type<fraction_type, bool_type> : common_type<fraction_type, int_type> {};

  // int

  template <> struct common_type<int_type, complex_type> : common_type<complex_type, int_type> {};
  template <> struct common_type<int_type, float_type> : common_type<float_type, int_type> {};
  template <> struct common_type<int_type, fraction_type> : common_type<fraction_type, int_type> {};
  template <> struct common_type<int_type, bool_type> { using type = int_type; };

  // bool

  template <> struct common_type<bool_type, complex_type> : common_type<complex_type, bool_type> {};
  template <> struct common_type<bool_type, float_type> : common_type<float_type, bool_type> {};
  template <> struct common_type<bool_type, fraction_type> : common_type<fraction_type, bool_type> {};
  template <> struct common_type<bool_type, int_type> : common_type<int_type, bool_type> {};
  template <> struct common_type<bool_type, bool_type> : common_type<int_type, bool_type> {};


  //
  // Missing operators
  //

  inline complex_type operator%(const complex_type& l, const complex_type& r) noexcept
  {
    const auto quotient = l / r;
    const auto mul = complex_type{ std::round(quotient.real()), std::round(quotient.imag()) };
    return l - mul * r;
  }
  inline float_type   operator%(const fraction_type& l, const fraction_type& r) noexcept
  {
    return std::fmod(l.to<float_type>(), r.to<float_type>());
  }


  //
  // Utility functions
  //

  namespace detail
  {
    template <typename T> concept has_invert = requires(T t) { utils::inv(t); };
    template <typename T> concept has_eq = requires(T t) { utils::eq(t, t); };
    template <typename T> concept has_less = requires(T t) { t < t; };
    template <typename T> concept has_abs = requires(T t) { utils::abs(t); };
  }

  template <typename T> auto abs(const T&) noexcept;
  inline auto abs(const detail::has_abs auto& op) noexcept
  {
    return utils::abs(op);
  }
  template <> inline auto abs(const bool_type& op) noexcept
  {
    return abs(static_cast<int_type>(op));
  }
  template <> inline auto abs(const complex_type& op) noexcept
  {
    const auto r = op.real();
    const auto i = op.imag();
    return std::sqrt(r * r + i * i);
  }
  template <> inline auto abs(const function_type& op) noexcept
  {
    return op;
  }

  template <typename T> auto inv(const T&) noexcept;
  inline auto inv(const detail::has_invert auto& val) noexcept
  {
    return utils::inv(val);
  }
  template <> inline auto inv(const int_type& i) noexcept
  {
    return inv(static_cast<float_type>(i));
  }
  template <> inline auto inv(const complex_type& cplx) noexcept
  {
    return 1.0 / cplx;
  }

  template <typename T> auto eq(const T&, const T&) noexcept;
  template <detail::has_eq T> inline auto eq(const T& lhs, const T& rhs) noexcept
  {
    return utils::eq(lhs, rhs);
  }
  template <> inline auto eq(const complex_type& lhs, const complex_type& rhs) noexcept
  {
    return eq(lhs.real(), rhs.real())
        && eq(lhs.imag(), rhs.imag());
  }
  template <> inline auto eq(const fraction_type& lhs, const fraction_type& rhs) noexcept
  {
    return lhs == rhs;
  }
  template <> inline auto eq(const function_type& lhs, const function_type& rhs) noexcept
  {
    return lhs == rhs;
  }

  template <typename T> auto less(const T&, const T&) noexcept;
  template <detail::has_less T> inline auto less(const T& lhs, const T& rhs) noexcept
  {
    return lhs < rhs;
  }
  template <> inline auto less(const complex_type& lhs, const complex_type& rhs) noexcept
  {
    return abs(lhs) < abs(rhs);
  }

  namespace detail
  {
    template <typename F, typename T>
    concept unary_function = std::is_nothrow_invocable_v<F, T>;

    template <typename F, typename T1, typename T2>
    concept binary_function = std::is_nothrow_invocable_v<F, T1, T2>;

    template <typename T>
    concept plusable = generic_type<T> &&
      requires(T v) { +v; };

    template <typename T>
    concept negatable = generic_type<T> &&
      requires(T v) { -v; };

    template <typename T>
    concept addable = generic_type<T> &&
      requires(T l, T r) { l + r; };

    template <typename T>
    concept subtractable = generic_type<T> &&
      requires(T l, T r) { l - r; };

    template <typename T>
    concept multipliable = generic_type<T> &&
      requires(T l, T r) { l* r; };

    template <typename T>
    concept divisible = generic_type<T> &&
      requires(T l, T r) { l / r; };

    template <typename T>
    concept modulo_divisible = generic_type<T> &&
      requires(T l, T r) { l% r; };

    template <typename T>
    concept fmod_divisible = generic_type<T> &&
      !std::integral<T> &&
      requires(T l, T r) { std::fmod(l, r); };

    template <typename T>
    concept pow_raisable = generic_type<T> &&
      requires(T l, T r) { std::pow(l, r); };

    template <typename T>
    concept invertible = generic_type<T> &&
      requires(T op) { eval::inv(op); };

    template <typename T>
    concept eq_comparable = generic_type<T> &&
      requires(T l, T r) { eval::eq(l, r); };

    template <typename T>
    concept rel_comparable = generic_type<T> &&
      requires(T l, T r) { eval::less(l, r); };

    template <typename T>
    concept fully_comparable = eq_comparable<T> && rel_comparable<T>;

    template <typename T>
    concept abs_compatible = generic_type<T> &&
      requires(T op) { eval::abs(op); };
  }
}