//
// Value visitor
//

#pragma once
#include "evaluator/value_registry.hpp"
#include "evaluator/traits.hpp"

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
    BitwiseAnd,
    BitwiseXor,
    BitwiseOr,
    BinaryPow,
    BinaryRoot,
    UnaryNegation,
    UnaryPlus,
    UnaryBitwiseNot
  };

  namespace detail
  {
    template <typename F, typename T>
    concept unary_function = std::is_nothrow_invocable_v<F, T>;

    template <typename F, typename T1, typename T2>
    concept binary_function = std::is_nothrow_invocable_v<F, T1, T2>;

    //
    // Helper object to facilitate easy casts from pointers to entity ids
    //
    struct ent_id
    {
      CLASS_SPECIALS_ALL(ent_id);
      using id_t = registry::entity_id;

      static consteval auto invalid_id() noexcept
      {
        return ~id_t{};
      }

      ent_id(const void* ent) noexcept :
        value{ reinterpret_cast<id_t>(ent) }
      {}

      ent_id(std::nullptr_t) noexcept :
        value{}
      {}

      auto operator* () const noexcept
      {
        return value;
      }

      id_t value{};
    };

    constexpr auto is_unary(val_ops op) noexcept
    {
      using enum val_ops;
      return utils::eq_any(op, UnaryPlus, UnaryNegation, UnaryBitwiseNot);
    }
    constexpr auto is_binary(val_ops op) noexcept
    {
      using enum val_ops;
      return utils::eq_any(op, Addition, Subtraction, 
                               Multiplication, Division, Modulo,
                               BitwiseAnd, BitwiseOr, BitwiseXor,
                               BinaryPow, BinaryRoot);
    }

  }

  //
  // Missing operators
  //
  inline complex_type operator+(const complex_type& l, int_type r) noexcept
  {
    return l + static_cast<float_type>(r);
  }
  inline complex_type operator+(int_type l, const complex_type& r) noexcept
  {
    return r + l;
  }
  inline complex_type operator+(const complex_type& l, const fraction_type& r) noexcept
  {
    return l + r.to<float_type>();
  }
  inline complex_type operator+(const fraction_type& l, const complex_type& r) noexcept
  {
    return r + l;
  }

  inline complex_type operator-(const complex_type& l, int_type r) noexcept
  {
    return l - static_cast<float_type>(r);
  }
  inline complex_type operator-(int_type l, const complex_type& r) noexcept
  {
    return static_cast<float_type>(l) - r;
  }
  inline complex_type operator-(const complex_type& l, const fraction_type& r) noexcept
  {
    return l - r.to<float_type>();
  }
  inline complex_type operator-(const fraction_type& l, const complex_type& r) noexcept
  {
    return l.to<float_type>() - r;
  }

  inline complex_type operator*(const complex_type& l, int_type r) noexcept
  {
    return l * static_cast<float_type>(r);
  }
  inline complex_type operator*(int_type l, const complex_type& r) noexcept
  {
    return r * l;
  }
  inline complex_type operator*(const complex_type& l, const fraction_type& r) noexcept
  {
    return l * r.to<float_type>();
  }
  inline complex_type operator*(const fraction_type& l, const complex_type& r) noexcept
  {
    return r * l;
  }

  inline complex_type operator/(const complex_type& l, int_type r) noexcept
  {
    return l / static_cast<float_type>(r);
  }
  inline complex_type operator/(int_type l, const complex_type& r) noexcept
  {
    return static_cast<float_type>(l) / r;
  }
  inline complex_type operator/(const complex_type& l, const fraction_type& r) noexcept
  {
    return l / r.to<float_type>();
  }
  inline complex_type operator/(const fraction_type& l, const complex_type& r) noexcept
  {
    return l.to<float_type>() / r;
  }

  template <detail::expr_result T> requires std::is_arithmetic_v<T>
  inline complex_type operator%(const complex_type& l, T r) noexcept
  {
    return { std::fmod(l.real(), static_cast<float_type>(r)),
             std::fmod(l.imag(), static_cast<float_type>(r)) };
  }
  template <detail::expr_result T> requires std::is_arithmetic_v<T>
  inline float_type operator%(const fraction_type& l, T r) noexcept
  {
    return std::fmod(l.to<float_type>(), static_cast<float_type>(r));
  }
  
  template <detail::expr_result T> requires std::is_arithmetic_v<T>
  inline float_type operator%(T l, const complex_type& r) noexcept
  {
    return std::fmod(static_cast<float_type>(l), r.real());
  }
  template <detail::expr_result T> requires std::is_arithmetic_v<T>
  inline float_type operator%(T l, const fraction_type& r) noexcept
  {
    return std::fmod(static_cast<float_type>(l), r.to<float_type>());
  }
  
  inline complex_type operator%(const complex_type& l, const complex_type& r) noexcept
  {
    const auto quotient = l / r;
    const auto mul = complex_type{ std::round(quotient.real()), std::round(quotient.imag()) };
    return l - mul * r;
  }
  inline float_type   operator%(const fraction_type& l, const fraction_type& r) noexcept
  {
    return l % r.to<float_type>();
  }

  inline complex_type operator%(const complex_type& l, const fraction_type& r) noexcept
  {
    return l % r.to<float_type>();
  }
  inline float_type   operator%(const fraction_type& l, const complex_type& r) noexcept
  {
    return l.to<float_type>() % r;
  }

  //
  // Value visitor used in expression evaluations
  //
  class value_visitor final
  {
  public:
    using enum val_ops;
    using id_param_t = detail::ent_id;
    using entity_id  = id_param_t::id_t;

  public:
    CLASS_SPECIALS_NONE(value_visitor);

    explicit value_visitor(registry& reg) noexcept :
      m_registry{ reg }
    {}

  private: // Invalid value handlers
    value visit_unary(invalid_val_t, val_ops) noexcept
    {
      return get_empty();
    }
    value visit_assign(invalid_val_t) noexcept
    {
      return get_empty();
    }
    value visit_binary(invalid_val_t, invalid_val_t, val_ops) noexcept
    {
      return get_empty();
    }
    template <typename T> value visit_binary(invalid_val_t, T, val_ops) noexcept
    {
      return visit_binary(invalid_val_t{}, invalid_val_t{}, val_ops{});
    }
    template <typename T> value visit_binary(T, invalid_val_t, val_ops) noexcept
    {
      return visit_binary(invalid_val_t{}, invalid_val_t{}, val_ops{});
    }

  private: // Operation support

    template <detail::expr_result T>
    typed_value<int_type> to_int(T) noexcept
    {
      return {};
    }

    template <>
    typed_value<int_type> to_int(int_type val) noexcept
    {
      return val;
    }

    template <>
    typed_value<int_type> to_int(float_type val) noexcept
    {
      const auto conv = static_cast<int_type>(val);
      if (utils::eq(static_cast<float_type>(conv), val))
        return conv;

      return {};
    }

    template <>
    typed_value<int_type> to_int(complex_type val) noexcept
    {
      if (!utils::eq(val.imag(), float_type{}))
        return {};

      return to_int(val.real());
    }

    template <>
    typed_value<int_type> to_int(fraction_type val) noexcept
    {
      return to_int(val.to<float_type>());
    }


    template <detail::expr_result T>
    typed_value<float_type> to_float(T) noexcept
    {
      return {};
    }

    template <>
    typed_value<float_type> to_float(int_type val) noexcept
    {
      return static_cast<float_type>(val);
    }

    template <>
    typed_value<float_type> to_float(float_type val) noexcept
    {
      return val;
    }

    template <>
    typed_value<float_type> to_float(complex_type val) noexcept
    {
      if (!utils::eq(val.imag(), float_type{}))
        return {};

      return to_float(val.real());
    }

    template <>
    typed_value<float_type> to_float(fraction_type val) noexcept
    {
      return val.to<float_type>();
    }


    // Addition
    
    template <detail::expr_result L, detail::expr_result R>
    auto add(L, R) noexcept { return get_empty(); }
    template <detail::expr_result L, detail::expr_result R>
      requires requires (L l, R r) { l + r; }
    auto add(L lhs, R rhs) noexcept
    {
      return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l + r; });
    }


    // Subtraction

    template <detail::expr_result L, detail::expr_result R>
    auto sub(L, R) noexcept { return get_empty(); }
    template <detail::expr_result L, detail::expr_result R>
      requires requires (L l, R r) { l - r; }
    auto sub(L lhs, R rhs) noexcept
    {
      return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l - r; });
    }


    // Multiplication

    template <detail::expr_result L, detail::expr_result R>
    auto mul(L, R) noexcept { return get_empty(); }
    template <detail::expr_result L, detail::expr_result R>
      requires requires (L l, R r) { l * r; }
    auto mul(L lhs, R rhs) noexcept
    {
      return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l * r; });
    }


    // Division

    template <detail::expr_result L, detail::expr_result R>
    auto div(L, R) noexcept { return get_empty(); }
    template <detail::expr_result L, detail::expr_result R>
      requires requires (L l, R r) { l / r; }
    auto div(L lhs, R rhs) noexcept
    {
      return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l / r; });
    }
    
    //
    // Corner case. In most cases, we don't want integral division, so let's convert lhs to float
    // if the remainder is not zero
    //
    auto div(int_type lhs, int_type rhs) noexcept
    {
      if(rhs && !(lhs % rhs))
        return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l / r; });

      return div(static_cast<float_type>(lhs), rhs);
    }


    // Modulo

    template <detail::expr_result L, detail::expr_result R>
    auto mod(L, R) noexcept { return get_empty(); }
    template <detail::expr_result L, detail::expr_result R>
      requires requires (L l, R r) { l % r; }
    auto mod(L lhs, R rhs) noexcept
    {
      return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l % r; });
    }
    template <detail::expr_result R>
      requires std::is_convertible_v<R, float_type>
    auto mod(float_type lhs, R rhs) noexcept
    {
      return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return std::fmod(l, r); });
    }
    template <detail::expr_result L>
      requires (std::is_convertible_v<L, float_type> && !is_same_noquals_v<L, float_type>)
    auto mod(L lhs, float_type rhs) noexcept
    {
      return mod(static_cast<float_type>(lhs), rhs);
    }

    // Corner case. If we have ints, and rhs is 0, we'll do a floating-point modulo
    // to get NAN instead of an exception
    auto mod(int_type lhs, int_type rhs) noexcept
    {
      return rhs ?
        mod<int_type, int_type>(lhs, rhs) :
        mod(static_cast<float_type>(lhs), rhs);
    }


    // Unary plus
    
    template <detail::expr_result T>
      requires requires (T v) { +v; }
    auto unary_plus(T operand) noexcept
    {
      return visit_unary(operand, [](auto val) noexcept { return +val; });
    }
    template <detail::expr_result T>
    auto unary_plus(T) noexcept
    {
      return get_empty();
    }

    // Unary minus

    template <detail::expr_result T>
      requires requires (T v) { -v; }
    auto unary_neg(T operand) noexcept
    {
      return visit_unary(operand, [](auto val) noexcept { return -val; });
    }
    template <detail::expr_result T>
    auto unary_neg(T) noexcept
    {
      return get_empty();
    }


    // Bitwise not

    template <detail::expr_result T>
      requires requires (T v) { ~v; }
    auto bitwise_not(T operand) noexcept
    {
      return visit_unary(operand, [](auto val) noexcept { return ~val; });
    }
    template <detail::expr_result T>
    auto bitwise_not(T operand) noexcept
    { 
      if (auto intOp = to_int(operand))
        return bitwise_not(*intOp);

      return get_empty();
    }

    // Bitwise and

    template <detail::expr_result L, detail::expr_result R>
      requires requires (L l, R r) { l & r; }
    auto bitwise_and(L lhs, R rhs) noexcept
    { 
      return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l & r; });
    }
    template <detail::expr_result L, detail::expr_result R>
    auto bitwise_and(L lhs, R rhs) noexcept
    {
      auto intL = to_int(lhs);
      auto intR = to_int(rhs);
      if (intL && intR)
        return bitwise_and(*intL, *intR);

      return get_empty(); 
    }

    // Bitwise xor

    template <detail::expr_result L, detail::expr_result R>
      requires requires (L l, R r) { l ^ r; }
    auto bitwise_xor(L lhs, R rhs) noexcept
    {
      return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l ^ r; });
    }
    template <detail::expr_result L, detail::expr_result R>
    auto bitwise_xor(L lhs, R rhs) noexcept
    {
      auto intL = to_int(lhs);
      auto intR = to_int(rhs);
      if (intL && intR)
        return bitwise_xor(*intL, *intR);

      return get_empty();
    }

    // Bitwise or

    template <detail::expr_result L, detail::expr_result R>
      requires requires (L l, R r) { l | r; }
    auto bitwise_or(L lhs, R rhs) noexcept
    {
      return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l | r; });
    }
    template <detail::expr_result L, detail::expr_result R>
    auto bitwise_or(L lhs, R rhs) noexcept
    {
      auto intL = to_int(lhs);
      auto intR = to_int(rhs);
      if (intL && intR)
        return bitwise_or(*intL, *intR);

      return get_empty();
    }

    // Power

    template <detail::expr_result L, detail::expr_result R>
    typed_value<complex_type> enforce_complex(L, R) noexcept
    {
      return {};
    }
    template <detail::expr_result L, detail::expr_result R>
      requires (std::is_arithmetic_v<L> && std::is_arithmetic_v<R>)
    typed_value<complex_type> enforce_complex(L l, R r) noexcept
    {
      auto base = static_cast<float_type>(l);
      if (base > 0.0 || utils::eq(base, 0.0))
        return {};

      auto exp  = static_cast<float_type>(r);
      if (!utils::in_range(exp, 0.0, 1.0))
        return {};

      if (const auto mod2 = std::fmod(utils::inv(exp), 2.0); !utils::eq(mod2, 0.0))
        return {};

      return complex_type{ 0.0, std::pow(utils::abs(base), exp) };
    }

    template <detail::expr_result L, detail::expr_result R>
      requires requires (L l, R r) { std::pow(l, r); }
    auto power(L base, R exp) noexcept
    {
      if (auto cpl = enforce_complex(base, exp))
      {
        return reg_value(*cpl);
      }

      return visit_binary(base, exp, [](auto l, auto r) noexcept
        {
          return std::pow(l, r);
        });
    }
    template <detail::expr_result L, detail::expr_result R>
    auto power(L base, R exp) noexcept
    {
      auto floatL = to_float(base);
      auto floatR = to_float(exp);
      if(floatL && floatR)
        return power(*floatL, *floatR);

      return get_empty();
    }
    auto power(fraction_type l, complex_type r) noexcept
    {
      return power(l.to<float_type>(), r);
    }
    auto power(complex_type l, fraction_type r) noexcept
    {
      return power(l, r.to<float_type>());
    }

    // Root
    template <detail::expr_result L, detail::expr_result R>
    auto root(L, R) noexcept { return get_empty(); }
    template <detail::expr_result L, detail::expr_result R>
      requires requires (R r) { 1 / r; }
    auto root(L base, R exp) noexcept
    {
      if constexpr (std::same_as<R, int_type>)
        return root(base, static_cast<float_type>(exp));
      else
        return power(base, 1 / exp);
    }


  private:
    //
    // Extracts type from value and calls the specified function
    //
    template <typename F>
    value visit_value(value val, F&& func) noexcept
    {
      return on_value(val, std::forward<F>(func));
    }

    //
    // Registers the value in the registry
    //
    template <detail::expr_result T>
    value reg_value(T val) noexcept
    {
      if (m_curEntity != invalidEnt)
        return m_registry.register_entity(m_curEntity, val);

      return m_registry.register_entity({}, val);
    }

    //
    // Registers result of assignment operations
    //
    template <detail::expr_result T>
    value visit_assign(T rhs) noexcept
    {
      return reg_value(rhs);
    }

    //
    // Registers result of unary operations
    //
    template <detail::expr_result T, detail::unary_function<T> F>
    value visit_unary(T val, F&& op) noexcept
    {
      return reg_value(op(val));
    }

    //
    // Dispatches unary operations according to operator type
    //
    template <detail::expr_result T>
    value visit_unary(T val, val_ops op) noexcept
    {
      using enum val_ops;
      switch (op)
      {
      case UnaryNegation:
        return unary_neg(val);

      case UnaryPlus:
        return unary_plus(val);

      case UnaryBitwiseNot:
        return bitwise_not(val);

      default:
        return get_empty();
      }
    }

    //
    // Registers result of binary operations
    //
    template <detail::expr_result L, detail::expr_result R, detail::binary_function<L, R> F>
    value visit_binary(L lhs, R rhs, F&& op) noexcept
    {
      return reg_value(op(lhs, rhs));
    }

    //
    // Intermadiate binary visitor
    // Dispatches the right operand according to its type
    //
    template <detail::expr_result L>
    value visit_binary(L lhs, value rhs, val_ops op) noexcept
    {
      return visit_value(rhs, [this, lhs, op](auto rhs) noexcept
        {
          return visit_binary(lhs, rhs, op);
        });
    }

    //
    // Dispatches binary operations according to operator type
    //
    template <detail::expr_result L, detail::expr_result R>
    value visit_binary(L lhs, R rhs, val_ops op) noexcept
    {
      using enum val_ops;
      switch (op)
      {
      case Addition:
        return add(lhs, rhs);

      case Subtraction:
        return sub(lhs, rhs);

      case Multiplication:
        return mul(lhs, rhs);

      case Division:
        return div(lhs, rhs);

      case Modulo:
        return mod(lhs, rhs);

      case BitwiseAnd:
        return bitwise_and(lhs, rhs);

      case BitwiseXor:
        return bitwise_xor(lhs, rhs);

      case BitwiseOr:
        return bitwise_or(lhs, rhs);

      case BinaryPow:
        return power(lhs, rhs);

      case BinaryRoot:
        return root(lhs, rhs);

      default:
        return get_empty();
      }
    }

    //
    // Dispatches the instantiation call
    //
    template <detail::expr_result Obj, typename T, T... Seq>
    value instantiate(const std::array<value, sizeof...(Seq)>& args, std::integer_sequence<T, Seq...>) noexcept
    {
      using type_info = eval::type_info<Obj>;
      using type_gen = type_wrapper<Obj>;
      const auto instance = type_gen{}(cast_value<utils::id_to_type_t<type_info::params[Seq]>>{}(args[Seq])...);

      if (!instance)
        return get_empty();

      return reg_value(*instance);
    }

  public:
    //
    // Instantiates an object
    //
    template <detail::expr_result Obj, typename... Args>
      requires is_all_v<value, Args...>
    value instantiate(id_param_t ent, Args ...args) noexcept
    {
      using type_info = eval::type_info<Obj>;
      static constexpr auto max = type_info::maxArgs;
      static_assert(sizeof ...(Args) == max);

      value_guard _{ m_curEntity, *ent };

      const std::array argList{ args... };
      return instantiate<Obj>(argList, std::make_index_sequence<max>{});
    }

    //
    // Stores a value for a function
    //
    value make_function(id_param_t ent, function_type f) noexcept
    {
      value_guard _{ m_curEntity, *ent };
      return reg_value(f);
    }

    //
    // Returns a resulting value from a binary expr
    //
    value visit_binary(id_param_t ent, value lhs, value rhs, val_ops op) noexcept
    {
      if (!lhs || !rhs || !detail::is_binary(op))
        return get_empty();

      value_guard _{ m_curEntity, *ent };

      return visit_value(lhs, [this, rhs, op](auto lhs) noexcept
        {
          return visit_binary(lhs, rhs, op);
        });
    }

    //
    // Returns a resulting value from a unary expr
    //
    value visit_unary(id_param_t ent, value val, val_ops op) noexcept
    {
      if (!val || !detail::is_unary(op))
        return get_empty();

      value_guard _{ m_curEntity, *ent };

      return visit_value(val, [this, op](auto v) noexcept
        {
          return visit_unary(v, op);
        });
    }

    //
    // Makes a value for an assigned-to entity
    //
    value visit_assign(id_param_t ent, value rhs) noexcept
    {
      if (!rhs)
        return get_empty();

      value_guard _{ m_curEntity, *ent };

      return visit_value(rhs, [this](auto v) noexcept
        {
          return visit_assign(v);
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
        return get_empty();

      return m_registry.register_literal(result);
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
        return get_empty();

      return m_registry.register_literal(result);
    }

    //
    // Retrieves the last evaluation result
    //
    value last_result(id_param_t ent) noexcept
    {
      auto ret = visit_assign(ent, m_registry.evaluation_result());
      return ret;
    }

    //
    // Resets the last evaluation result and returns an empty value
    //
    value get_empty() noexcept
    {
      return m_registry.reset_result();
    }

  private:
    static constexpr auto invalidEnt = detail::ent_id::invalid_id();
    entity_id m_curEntity{ invalidEnt };
    registry& m_registry;
  };
}