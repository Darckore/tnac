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
    UnaryBitwiseNot,
    LogicalNot
  };

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
      return utils::eq_any(op, UnaryPlus, UnaryNegation, UnaryBitwiseNot, LogicalNot);
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


  private: // Unary operations

    auto unary_plus(detail::plusable auto operand) noexcept
    {
      return visit_unary(operand, [](auto val) noexcept { return +val; });
    }
    auto unary_plus(bool v) noexcept
    {
      return unary_plus(static_cast<int_type>(v));
    }
    template <detail::generic_type T> 
    auto unary_plus(T) noexcept
    {
      return get_empty();
    }

    auto unary_neg(detail::negatable auto operand) noexcept
    {
      return visit_unary(operand, [](auto val) noexcept { return -val; });
    }
    auto unary_neg(bool v) noexcept
    {
      return unary_neg(static_cast<int_type>(v));
    }
    template <detail::generic_type T>
    auto unary_neg(T) noexcept
    {
      return get_empty();
    }

    template <detail::generic_type T>
    auto bitwise_not(T operand) noexcept
    {
      if (auto intOp = get_caster<int_type>()(operand))
        return visit_unary(*intOp, [](auto val) noexcept { return ~val; });

      return get_empty();
    }

    template <detail::generic_type T>
    auto logical_not(T operand) noexcept
    {
      auto boolOp = get_caster<bool_type>()(operand);
      return visit_unary(boolOp && *boolOp, [](auto val) noexcept { return !val; });
    }


  private: // Binary arithmetic operations

    // Addition

    template <detail::generic_type L, detail::generic_type R>
    auto add(L, R) noexcept { return get_empty(); }
    template <detail::generic_type L, detail::generic_type R>
      requires (!is_any_v<bool_type, L, R> && requires (L l, R r) { l + r; })
    auto add(L lhs, R rhs) noexcept
    {
      return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l + r; });
    }
    template <detail::generic_type L>
      requires (!is_same_noquals_v<L, bool_type>)
    auto add(L lhs, bool_type rhs) noexcept
    {
      return add(lhs, static_cast<int_type>(rhs));
    }
    template <detail::generic_type R>
    auto add(bool_type lhs, R rhs) noexcept
    {
      return add(static_cast<int_type>(lhs), rhs);
    }


    // Subtraction

    template <detail::generic_type L, detail::generic_type R>
    auto sub(L, R) noexcept { return get_empty(); }
    template <detail::generic_type L, detail::generic_type R>
      requires (!is_any_v<bool_type, L, R> && requires (L l, R r) { l - r; })
    auto sub(L lhs, R rhs) noexcept
    {
      return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l - r; });
    }
    template <detail::generic_type L>
      requires (!is_same_noquals_v<L, bool_type>)
    auto sub(L lhs, bool_type rhs) noexcept
    {
      return sub(lhs, static_cast<int_type>(rhs));
    }
    template <detail::generic_type R>
    auto sub(bool_type lhs, R rhs) noexcept
    {
      return sub(static_cast<int_type>(lhs), rhs);
    }


    // Multiplication

    template <detail::generic_type L, detail::generic_type R>
    auto mul(L, R) noexcept { return get_empty(); }
    template <detail::generic_type L, detail::generic_type R>
      requires (!is_any_v<bool_type, L, R> && requires (L l, R r) { l * r; })
    auto mul(L lhs, R rhs) noexcept
    {
      return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l * r; });
    }
    template <detail::generic_type L>
      requires (!is_same_noquals_v<L, bool_type>)
    auto mul(L lhs, bool_type rhs) noexcept
    {
      return mul(lhs, static_cast<int_type>(rhs));
    }
    template <detail::generic_type R>
    auto mul(bool_type lhs, R rhs) noexcept
    {
      return mul(static_cast<int_type>(lhs), rhs);
    }


    // Division

    template <detail::generic_type L, detail::generic_type R>
    auto div(L, R) noexcept { return get_empty(); }
    template <detail::generic_type L, detail::generic_type R>
      requires (!is_any_v<bool_type, L, R> && requires (L l, R r) { l / r; })
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
    template <detail::generic_type L>
      requires (!is_same_noquals_v<L, bool_type>)
    auto div(L lhs, bool_type rhs) noexcept
    {
      return div(lhs, static_cast<int_type>(rhs));
    }
    template <detail::generic_type R>
    auto div(bool_type lhs, R rhs) noexcept
    {
      return div(static_cast<int_type>(lhs), rhs);
    }


    // Modulo

    template <detail::generic_type L, detail::generic_type R>
    auto mod(L, R) noexcept { return get_empty(); }
    template <detail::generic_type L, detail::generic_type R>
      requires (!is_any_v<bool_type, L, R> && requires (L l, R r) { l % r; })
    auto mod(L lhs, R rhs) noexcept
    {
      return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return l % r; });
    }
    template <detail::generic_type R>
      requires std::is_convertible_v<R, float_type>
    auto mod(float_type lhs, R rhs) noexcept
    {
      return visit_binary(lhs, rhs, [](auto l, auto r) noexcept { return std::fmod(l, r); });
    }
    template <detail::generic_type L>
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



    // Bitwise and

    template <detail::generic_type L, detail::generic_type R>
    auto bitwise_and(L lhs, R rhs) noexcept
    {
      auto caster = get_caster<int_type>();
      auto intL = caster(lhs);
      auto intR = caster(rhs);
      if (intL && intR)
        return visit_binary(*intL, *intR, [](auto l, auto r) noexcept { return l & r; });

      return get_empty(); 
    }


    // Bitwise xor

    template <detail::generic_type L, detail::generic_type R>
    auto bitwise_xor(L lhs, R rhs) noexcept
    {
      auto caster = get_caster<int_type>();
      auto intL = caster(lhs);
      auto intR = caster(rhs);
      if (intL && intR)
        return visit_binary(*intL, *intR, [](auto l, auto r) noexcept { return l ^ r; });

      return get_empty();
    }


    // Bitwise or

    template <detail::generic_type L, detail::generic_type R>
    auto bitwise_or(L lhs, R rhs) noexcept
    {
      auto caster = get_caster<int_type>();
      auto intL = caster(lhs);
      auto intR = caster(rhs);
      if (intL && intR)
        return visit_binary(*intL, *intR, [](auto l, auto r) noexcept { return l | r; });

      return get_empty();
    }


    // Power

    template <detail::generic_type L, detail::generic_type R>
    typed_value<complex_type> enforce_complex(L, R) noexcept
    {
      return {};
    }
    template <detail::generic_type L, detail::generic_type R>
      requires (std::is_arithmetic_v<L> && std::is_arithmetic_v<R>)
    typed_value<complex_type> enforce_complex(L l, R r) noexcept
    {
      auto base = static_cast<float_type>(l);
      if (base > 0.0 || utils::eq(base, 0.0))
        return {};

      auto exp  = static_cast<float_type>(r);
      if (const auto mod2 = std::fmod(utils::inv(exp), 2.0); !utils::eq(mod2, 0.0))
        return {};

      return complex_type{ 0.0, std::pow(utils::abs(base), exp) };
    }

    template <detail::generic_type L, detail::generic_type R>
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
    template <detail::generic_type L, detail::generic_type R>
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

    template <detail::generic_type L, detail::generic_type R>
    auto root(L, R) noexcept { return get_empty(); }
    template <detail::generic_type L, detail::generic_type R>
      requires requires (R r) { 1 / r; }
    auto root(L base, R exp) noexcept
    {
      if constexpr (is_any_v<R, int_type, bool_type>)
        return root(base, static_cast<float_type>(exp));
      else
        return power(base, 1 / exp);
    }
    template <detail::generic_type R>
    auto root(bool_type base, R exp) noexcept
    {
      return root(static_cast<int_type>(base), exp);
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
    // Resets the result and returns an empty value
    //
    value reg_value(invalid_val_t) noexcept
    {
      return get_empty();
    }

    //
    // Registers the value in the registry
    //
    template <detail::expr_result T>
    value reg_value(T val) noexcept
    {
      if (m_curEntity != invalidEnt)
        return m_registry.register_entity(m_curEntity, std::move(val));

      return m_registry.register_entity({}, std::move(val));
    }

    //
    // Registers result of assignment operations
    //
    template <detail::generic_type T>
    value visit_assign(T rhs) noexcept
    {
      return reg_value(std::move(rhs));
    }

    //
    // Registers result of unary operations
    //
    template <detail::generic_type T, detail::unary_function<T> F>
    value visit_unary(T val, F&& op) noexcept
    {
      return reg_value(op(std::move(val)));
    }

    //
    // Dispatches unary operations according to operator type
    //
    template <detail::generic_type T>
    value visit_unary(T val, val_ops op) noexcept
    {
      using enum val_ops;
      switch (op)
      {
      case UnaryNegation:
        return unary_neg(std::move(val));

      case UnaryPlus:
        return unary_plus(std::move(val));

      case UnaryBitwiseNot:
        return bitwise_not(std::move(val));

      case LogicalNot:
        return logical_not(std::move(val));

      default:
        return get_empty();
      }
    }

    //
    // Registers result of binary operations
    //
    template <detail::generic_type L, detail::generic_type R, detail::binary_function<L, R> F>
    value visit_binary(L lhs, R rhs, F&& op) noexcept
    {
      return reg_value(op(std::move(lhs), std::move(rhs)));
    }

    //
    // Intermadiate binary visitor
    // Dispatches the right operand according to its type
    //
    template <detail::generic_type L>
    value visit_binary(L lhs, value rhs, val_ops op) noexcept
    {
      return visit_value(rhs, [this, lhs, op](auto rhs) noexcept
        {
          return visit_binary(std::move(lhs), std::move(rhs), op);
        });
    }

    //
    // Dispatches binary operations according to operator type
    //
    template <detail::generic_type L, detail::generic_type R>
    value visit_binary(L lhs, R rhs, val_ops op) noexcept
    {
      using enum val_ops;
      switch (op)
      {
      case Addition:
        return add(std::move(lhs), std::move(rhs));

      case Subtraction:
        return sub(std::move(lhs), std::move(rhs));

      case Multiplication:
        return mul(std::move(lhs), std::move(rhs));

      case Division:
        return div(std::move(lhs), std::move(rhs));

      case Modulo:
        return mod(std::move(lhs), std::move(rhs));

      case BitwiseAnd:
        return bitwise_and(std::move(lhs), std::move(rhs));

      case BitwiseXor:
        return bitwise_xor(std::move(lhs), std::move(rhs));

      case BitwiseOr:
        return bitwise_or(std::move(lhs), std::move(rhs));

      case BinaryPow:
        return power(std::move(lhs), std::move(rhs));

      case BinaryRoot:
        return root(std::move(lhs), std::move(rhs));

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
      auto instance = type_gen{}(cast_value<utils::id_to_type_t<type_info::params[Seq]>>(args[Seq])...);

      if (!instance)
        return get_empty();

      return reg_value(std::move(*instance));
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
      if (!detail::is_binary(op))
        return get_empty();

      value_guard _{ m_curEntity, *ent };

      return visit_value(lhs, [this, rhs, op](auto lhs) noexcept
        {
          return visit_binary(std::move(lhs), std::move(rhs), op);
        });
    }

    //
    // Returns a resulting value from a unary expr
    //
    value visit_unary(id_param_t ent, value val, val_ops op) noexcept
    {
      if (!detail::is_unary(op))
        return get_empty();

      value_guard _{ m_curEntity, *ent };

      return visit_value(val, [this, op](auto v) noexcept
        {
          return visit_unary(std::move(v), op);
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
    // Registers a boolean literal
    //
    value visit_bool_literal(bool value) noexcept
    {
      return m_registry.register_literal(value);
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