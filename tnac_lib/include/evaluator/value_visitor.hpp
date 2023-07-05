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

    template <typename T>
    concept addable = generic_type<T> &&
      requires(T l, T r) { l + r; };

    template <typename T>
    concept subtractable = generic_type<T> &&
      requires(T l, T r) { l - r; };

    template <typename T>
    concept multipliable = generic_type<T> &&
      requires(T l, T r) { l * r; };

    template <typename T>
    concept divisible = generic_type<T> &&
      requires(T l, T r) { l / r; };

    template <typename T>
    concept modulo_divisible = generic_type<T> &&
      requires(T l, T r) { l % r; };

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

      ent_id(id_t id) noexcept :
        value{ id }
      {}

      ent_id(const void* ent) noexcept :
        ent_id{ reinterpret_cast<id_t>(ent) }
      {}

      ent_id(std::nullptr_t) noexcept :
        ent_id{}
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
                               BinaryPow, BinaryRoot,
                               RelLess, RelLessEq, RelGr, RelGrEq,
                               Equal, NEqual);
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

    //
    // Registers result of unary operations
    //
    template <detail::generic_type T, detail::unary_function<T> F>
    value visit_unary(T val, F&& op) noexcept
    {
      return reg_value(op(std::move(val)));
    }

    auto unary_plus(detail::plusable auto operand) noexcept
    {
      return visit_unary(operand, [](auto val) noexcept { return +val; });
    }
    template <detail::generic_type T> 
    auto unary_plus(T) noexcept { return get_empty(); }

    auto unary_neg(detail::negatable auto operand) noexcept
    {
      return visit_unary(operand, [](auto val) noexcept { return -val; });
    }
    template <detail::generic_type T>
    auto unary_neg(T) noexcept { return get_empty(); }

    template <detail::generic_type T>
    auto bitwise_not(T operand) noexcept
    {
      if (auto intOp = get_caster<int_type>()(std::move(operand)))
        return visit_unary(*intOp, [](auto val) noexcept { return ~val; });

      return get_empty();
    }

    template <detail::generic_type T>
    auto logical_not(T operand) noexcept
    {
      auto boolOp = get_caster<bool_type>()(std::move(operand));
      return visit_unary(boolOp.value_or(false), [](auto val) noexcept { return !val; });
    }

    //
    // Dispatches unary operations according to operator type
    //
    template <detail::generic_type T>
    value visit_unary(T operand, val_ops op) noexcept
    {
      using op_type = common_type_t<T, T>;
      auto val = get_caster<op_type>()(std::move(operand));
      if (!val)
      {
        UTILS_ASSERT(false);
        return get_empty();
      }

      using enum val_ops;
      switch (op)
      {
      case UnaryNegation:   return unary_neg(std::move(*val));
      case UnaryPlus:       return unary_plus(std::move(*val));
      case UnaryBitwiseNot: return bitwise_not(std::move(*val));
      case LogicalNot:      return logical_not(std::move(*val));

      default: return get_empty();
      }
    }


  private: // Binary operations

    //
    // Registers result of binary operations
    //
    template <detail::generic_type L, detail::generic_type R, detail::binary_function<L, R> F>
    value visit_binary(L lhs, R rhs, F&& op) noexcept
    {
      return reg_value(op(std::move(lhs), std::move(rhs)));
    }

    // Bitwise

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

    // Simple arithmetic

    template <detail::addable T>
    auto add(T lhs, T rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return l + r;
        });
    }
    template <detail::generic_type T>
    auto add(T, T) noexcept { return get_empty(); }

    template <detail::subtractable T>
    auto sub(T lhs, T rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return l - r;
        });
    }
    template <detail::generic_type T>
    auto sub(T, T) noexcept { return get_empty(); }

    template <detail::multipliable T>
    auto mul(T lhs, T rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return l * r;
        });
    }
    template <detail::generic_type T>
    auto mul(T, T) noexcept { return get_empty(); }

    template <detail::divisible T>
    auto div(T lhs, T rhs) noexcept
    {
      if constexpr (is_same_noquals_v<T, int_type>)
      {
        return div(static_cast<float_type>(lhs), static_cast<float_type>(rhs));
      }
      else
      {
        return visit_binary(std::move(lhs), std::move(rhs),
          [](auto l, auto r) noexcept
          {
            return l / r;
          });
      }
    }
    template <detail::generic_type T>
    auto div(T, T) noexcept { return get_empty(); }

    // Modulo

    template <detail::fmod_divisible T>
    auto mod(T lhs, T rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return std::fmod(l, r);
        });
    }
    template <detail::modulo_divisible T>
    auto mod(T lhs, T rhs) noexcept
    {
      if constexpr (is_same_noquals_v<T, int_type>)
      {
        return mod(static_cast<float_type>(lhs), static_cast<float_type>(rhs));
      }
      else
      {
        return visit_binary(std::move(lhs), std::move(rhs),
          [](auto l, auto r) noexcept
          {
            return l % r;
          });
      }
    }
    template <detail::generic_type T>
    auto mod(T, T) noexcept { return get_empty(); }

    // Pow and root

    template <detail::generic_type T>
      requires (std::is_arithmetic_v<T>)
    auto enforce_complex(const T& l, const T& r) noexcept -> typed_value<complex_type>
    {
      auto base = static_cast<float_type>(l);
      if (base > 0.0 || utils::eq(base, 0.0))
        return {};

      auto exp = static_cast<float_type>(r);
      if (const auto mod2 = std::fmod(utils::inv(exp), 2.0); !utils::eq(mod2, 0.0))
        return {};

      return complex_type{ 0.0, std::pow(utils::abs(base), exp) };
    }
    template <detail::generic_type T>
    auto enforce_complex(const T&, const T&) noexcept -> typed_value<complex_type> { return {}; }

    template <detail::pow_raisable T>
    auto power(T base, T exp) noexcept
    {
      if (auto cpl = enforce_complex(base, exp))
      {
        return reg_value(*cpl);
      }

      return visit_binary(std::move(base), std::move(exp),
        [](auto l, auto r) noexcept
        {
          return std::pow(l, r);
        });
    }
    template <detail::generic_type T>
    auto power(T base, T exp) noexcept
    {
      auto caster = get_caster<float_type>();
      auto floatL = caster(base);
      auto floatR = caster(exp);
      if (floatL && floatR)
        return power(*floatL, *floatR);

      return get_empty();
    }

    template <detail::invertible T>
    auto root(T base, T exp) noexcept
    {
      if constexpr (is_same_noquals_v<T, int_type>)
        return root(static_cast<float_type>(base), static_cast<float_type>(exp));
      else
        return power(base, eval::inv(exp));
    }
    template <detail::generic_type T>
    auto root(T, T) noexcept { return get_empty(); }

    // Relation and equality

    template <detail::eq_comparable T>
    auto equal(T lhs, T rhs, bool compareForEquality) noexcept
    {
      const auto cmp = eval::eq(lhs, rhs);
      const auto res = compareForEquality ? cmp : !cmp;
      return reg_value(res);
    }
    template <detail::generic_type T>
    auto equal(T, T, bool) noexcept { return get_empty(); }

    template <detail::rel_comparable T>
    auto less(T lhs, T rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return eval::less(l, r);
        });
    }
    template <detail::generic_type T>
    auto less(T, T) noexcept { return get_empty(); }

    template <detail::fully_comparable T>
    auto less_eq(T lhs, T rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return eval::eq(l, r) || eval::less(l, r);
        });
    }
    template <detail::generic_type T>
    auto less_eq(T, T) noexcept { return get_empty(); }

    template <detail::fully_comparable T>
    auto greater(T lhs, T rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return !eval::eq(l, r) && !eval::less(l, r);
        });
    }
    template <detail::generic_type T>
    auto greater(T, T) noexcept { return get_empty(); }

    template <detail::fully_comparable T>
    auto greater_eq(T lhs, T rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return !eval::less(l, r);
        });
    }
    template <detail::generic_type T>
    auto greater_eq(T, T) noexcept { return get_empty(); }

    //
    // Dispatches binary operations according to operator type
    //
    template <detail::generic_type L, detail::generic_type R>
    value visit_binary(L l, R r, val_ops op) noexcept
    {
      using common_t = common_type_t<L, R>;
      auto caster = get_caster<common_t>();
      auto lhs = caster(std::move(l));
      auto rhs = caster(std::move(r));
      if (!lhs || !rhs)
        return get_empty();

      using enum val_ops;
      switch (op)
      {
      case Addition:       return add(std::move(*lhs), std::move(*rhs));
      case Subtraction:    return sub(std::move(*lhs), std::move(*rhs));
      case Multiplication: return mul(std::move(*lhs), std::move(*rhs));
      case Division:       return div(std::move(*lhs), std::move(*rhs));
      case Modulo:         return mod(std::move(*lhs), std::move(*rhs));

      case RelLess:   return less(std::move(*lhs), std::move(*rhs));
      case RelLessEq: return less_eq(std::move(*lhs), std::move(*rhs));
      case RelGr:     return greater(std::move(*lhs), std::move(*rhs));
      case RelGrEq:   return greater_eq(std::move(*lhs), std::move(*rhs));
      case Equal:     return equal(std::move(*lhs), std::move(*rhs), true);
      case NEqual:    return equal(std::move(*lhs), std::move(*rhs), false);

      case BitwiseAnd: return bitwise_and(std::move(*lhs), std::move(*rhs));
      case BitwiseXor: return bitwise_xor(std::move(*lhs), std::move(*rhs));
      case BitwiseOr:  return bitwise_or(std::move(*lhs), std::move(*rhs));

      case BinaryPow:  return power(std::move(*lhs), std::move(*rhs));
      case BinaryRoot: return root(std::move(*lhs), std::move(*rhs));

      default: return get_empty();
      }
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
    // Intermediate binary visitor
    // Dispatches the right operand according to its type
    //
    template <detail::generic_type L>
    value visit_binary(L lhs, value rhs, val_ops op) noexcept
    {
      return visit_value(rhs, [this, l = std::move(lhs), op](auto rhs) noexcept
        {
          return visit_binary(std::move(l), std::move(rhs), op);
        });
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

    //
    // Erases an entity's value from the registry
    //
    value remove_entity(id_param_t ent) noexcept
    {
      return m_registry.erase(*ent);
    }

  private:
    static constexpr auto invalidEnt = detail::ent_id::invalid_id();
    entity_id m_curEntity{ invalidEnt };
    registry& m_registry;
  };
}