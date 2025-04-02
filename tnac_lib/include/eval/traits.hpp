//
// Type traits
//

#pragma once
#include "eval/value.hpp"

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

  template <expr_result T> using typed_value = std::optional<T>;

  template <expr_result T> struct type_wrapper
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

  template <expr_result T>
  auto cast_value(old_crap_value val) noexcept { return on_value(val, get_caster<T>()); };

  inline auto to_bool(old_crap_value val) noexcept
  {
    auto resVal = cast_value<bool_type>(val);
    UTILS_ASSERT(resVal.has_value());
    return *resVal;
  }

  //
  // Common type
  //

  template <expr_result T1, expr_result T2> struct common_type;
  template <expr_result T1, expr_result T2> using common_type_t = typename common_type<T1, T2>::type;

  // common type with self

  template <expr_result T1, expr_result T2> requires (utils::same_noquals<T1, T2>)
  struct common_type<T1, T2> { using type = std::remove_cvref_t<T1>; };

  // invalid

  template <expr_result T> requires (!utils::same_noquals<T, invalid_val_t>)
  struct common_type<invalid_val_t, T> { using type = invalid_val_t; };
  template <expr_result T> requires (!utils::same_noquals<T, invalid_val_t>)
  struct common_type<T, invalid_val_t> : common_type<invalid_val_t, T> {};

  // array

  template <expr_result T> requires (!utils::any_same_as<T, array_type, invalid_val_t>)
  struct common_type<array_type, T> { using type = array_type; };
  template <expr_result T> requires (!utils::any_same_as<T, array_type, invalid_val_t>)
  struct common_type<T, array_type> : common_type<array_type, T> {};

  // function

  template <expr_result T> requires (!utils::any_same_as<T, function_type, invalid_val_t>)
  struct common_type<function_type, T> { using type = function_type; };
  template <expr_result T> requires (!utils::any_same_as<T, function_type, invalid_val_t>)
  struct common_type<T, function_type> : common_type<function_type, T> {};
  template <> struct common_type<function_type, array_type> { using type = array_type; };
  template <> struct common_type<array_type, function_type> : common_type<function_type, array_type> {};

  // complex

  template <> struct common_type<complex_type, float_type> { using type = complex_type; };
  template <> struct common_type<complex_type, fraction_type> : common_type<complex_type, float_type> {};
  template <> struct common_type<complex_type, int_type> : common_type<complex_type, float_type> {};
  template <> struct common_type<complex_type, bool_type> : common_type<complex_type, float_type> {};

  // float

  template <> struct common_type<float_type, complex_type> : common_type<complex_type, float_type> {};
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

  // Common type id

  namespace detail
  {
    template <type_id L, type_id R>
    constexpr auto ct_id() noexcept
    {
      using ltype = utils::id_to_type_t<L>;
      using rtype = utils::id_to_type_t<R>;
      using ctype = common_type_t<ltype, rtype>;
      return utils::type_to_id_v<ctype>;
    }

    template <type_id L>
    constexpr auto ct_id(type_id r) noexcept
    {
      using enum type_id;
      switch (r)
      {
      case Bool:     return ct_id<L, Bool>();
      case Int:      return ct_id<L, Int>();
      case Float:    return ct_id<L, Float>();
      case Complex:  return ct_id<L, Complex>();
      case Fraction: return ct_id<L, Fraction>();
      case Function: return ct_id<L, Function>();
      case Array:    return ct_id<L, Array>();
      default:       return Invalid;
      }
    }
  }

  constexpr auto common_type_id(type_id l, type_id r) noexcept
  {
    using enum type_id;
    switch (l)
    {
    case Bool:     return detail::ct_id<Bool>(r);
    case Int:      return detail::ct_id<Int>(r);
    case Float:    return detail::ct_id<Float>(r);
    case Complex:  return detail::ct_id<Complex>(r);
    case Fraction: return detail::ct_id<Fraction>(r);
    case Function: return detail::ct_id<Function>(r);
    case Array:    return detail::ct_id<Array>(r);
    default:       return Invalid;
    }
  }


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


  template <typename T> concept has_invert = requires(T t) { utils::inv(t); };
  template <typename T> concept has_eq = requires(T t) { utils::eq(t, t); };
  template <typename T> concept has_less = requires(T t) { t < t; };
  template <typename T> concept has_abs = requires(T t) { utils::abs(t); };

  template <typename T> auto abs(const T&) noexcept;
  inline auto abs(const has_abs auto& op) noexcept
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
  inline auto inv(const has_invert auto& val) noexcept
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
  template <has_eq T> inline auto eq(const T& lhs, const T& rhs) noexcept
  {
    if constexpr (utils::same_noquals<T, float_type>)
    {
      if (std::isinf(lhs) && std::isinf(rhs))
        return true;

      if (std::isnan(lhs) && std::isnan(rhs))
        return true;
    }

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
  template <has_less T> inline auto less(const T& lhs, const T& rhs) noexcept
  {
    return lhs < rhs;
  }
  template <> inline auto less(const complex_type& lhs, const complex_type& rhs) noexcept
  {
    return abs(lhs) < abs(rhs);
  }

  template <typename F, typename T>
  concept unary_function = std::is_nothrow_invocable_v<F, T>;
  
  template <typename F, typename T1, typename T2>
  concept binary_function = std::is_nothrow_invocable_v<F, T1, T2>;
  
  template <typename T>
  concept plusable = expr_result<T> &&
    requires(T v) { +v; };
  
  template <typename T>
  concept negatable = expr_result<T> &&
    requires(T v) { -v; };
  
  template <typename T>
  concept addable = expr_result<T> &&
    requires(T l, T r) { l + r; };
  
  template <typename T>
  concept subtractable = expr_result<T> &&
    requires(T l, T r) { l - r; };
  
  template <typename T>
  concept multipliable = expr_result<T> &&
    requires(T l, T r) { l * r; };
  
  template <typename T>
  concept divisible = expr_result<T> &&
    requires(T l, T r) { l / r; };
  
  template <typename T>
  concept modulo_divisible = expr_result<T> &&
    requires(T l, T r) { l % r; };
  
  template <typename T>
  concept fmod_divisible = expr_result<T> &&
    !std::integral<T> &&
    requires(T l, T r) { std::fmod(l, r); };
  
  template <typename T>
  concept pow_raisable = expr_result<T> &&
    requires(T l, T r) { std::pow(l, r); };
  
  template <typename T>
  concept invertible = expr_result<T> &&
    requires(T op) { eval::inv(op); };
  
  template <typename T>
  concept eq_comparable = expr_result<T> &&
    requires(T l, T r) { eval::eq(l, r); };
  
  template <typename T>
  concept rel_comparable = expr_result<T> &&
    requires(T l, T r) { eval::less(l, r); };
  
  template <typename T>
  concept fully_comparable = eq_comparable<T> && rel_comparable<T>;
  
  template <typename T>
  concept abs_compatible = expr_result<T> &&
    requires(T op) { eval::abs(op); };


  template <typename T>
  concept value_container = requires (T& t, std::size_t i)
  {
    { t[i] } -> std::same_as<stored_value&>;
  };


  //
  // Size of a type in bytes
  //

  template <expr_result Type>
  constexpr auto size_of() noexcept
  {
    return sizeof(Type);
  }

  template <>
  constexpr auto size_of<invalid_val_t>() noexcept
  {
    return decltype(sizeof(0)){};
  }

  template <>
  constexpr auto size_of<complex_type>() noexcept
  {
    return 2 * size_of<float_type>();
  }

  template <>
  constexpr auto size_of<fraction_type>() noexcept
  {
    return 2 * size_of<int_type>() + 1;
  }

  template <type_id Type>
  constexpr auto size_of() noexcept
  {
    return size_of<utils::id_to_type_t<Type>>();
  }

  constexpr auto size_of(type_id type) noexcept
  {
    using enum type_id;
    switch (type)
    {
    case Bool:      return size_of<Bool>();
    case Int:       return size_of<Int>();
    case Float:     return size_of<Float>();
    case Complex:   return size_of<Complex>();
    case Fraction:  return size_of<Fraction>();
    case Function:  return size_of<Function>();
    case Array:     return size_of<Array>();
    default:        return size_of<Invalid>();
    }
  }
}