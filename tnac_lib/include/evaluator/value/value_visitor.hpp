//
// Value visitor
//

#pragma once
#include "evaluator/value/value_registry.hpp"
#include "evaluator/type_support/traits.hpp"

namespace tnac::eval
{
  namespace detail
  {
    //
    // Helper object to facilitate easy casts from pointers to entity ids
    //
    struct ent_id
    {
      CLASS_SPECIALS_ALL(ent_id);
      using id_t = registry::entity_id;

      static consteval auto invalid_id() noexcept { return ~id_t{}; }

      ent_id(id_t id) noexcept :         value{ id } {}
      ent_id(const void* ent) noexcept : ent_id{ reinterpret_cast<id_t>(ent) } {}
      ent_id(std::nullptr_t) noexcept :  ent_id{} {}

      auto operator*() const noexcept { return value; }

      id_t value{};
    };

    constexpr auto is_unary(val_ops op) noexcept
    {
      using enum val_ops;
      return utils::eq_any(op, UnaryPlus, UnaryNegation, UnaryBitwiseNot,
                               LogicalNot, LogicalIs, AbsoluteValue);
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
    using arr_t      = registry::val_array;
    using size_type  = registry::size_type;

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
      return visit_unary(std::move(operand), [](auto val) noexcept { return +val; });
    }
    auto unary_plus(detail::generic_type auto) noexcept { return get_empty(); }

    auto unary_neg(detail::negatable auto operand) noexcept
    {
      return visit_unary(operand, [](auto val) noexcept { return -val; });
    }
    auto unary_neg(detail::generic_type auto) noexcept { return get_empty(); }

    auto bitwise_not(detail::generic_type auto operand) noexcept
    {
      if (auto intOp = get_caster<int_type>()(std::move(operand)))
        return visit_unary(*intOp, [](auto val) noexcept { return ~val; });

      return get_empty();
    }

    auto logical_not(detail::generic_type auto operand) noexcept
    {
      auto boolOp = get_caster<bool_type>()(std::move(operand));
      return visit_unary(boolOp.value_or(false), [](auto val) noexcept { return !val; });
    }

    auto logical_is(detail::generic_type auto operand) noexcept
    {
      auto boolOp = get_caster<bool_type>()(std::move(operand));
      return visit_unary(boolOp && *boolOp, [](auto val) noexcept { return val; });
    }

    auto abs(detail::abs_compatible auto operand) noexcept
    {
      return visit_unary(std::move(operand), [](auto val) noexcept
        {
          return eval::abs(val);
        });
    }
    auto abs(detail::generic_type auto) noexcept { return get_empty(); }

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
      case LogicalIs:       return logical_is(std::move(*val));
      case AbsoluteValue:   return abs(std::move(*val));

      default: return get_empty();
      }
    }

    template <>
    value visit_unary(array_type operand, val_ops op) noexcept
    {
      if (utils::eq_any(op, val_ops::LogicalIs, val_ops::LogicalNot))
      {
        auto toBool = get_caster<bool_type>()(std::move(operand));
        return visit_unary(toBool.value_or(false), op);
      }

      auto&& newArr = m_registry.allocate_array(m_curEntity, operand->size());
      const auto arrEnt = *id_param_t{ &newArr };
      for (auto idx = size_type{}; auto el : *operand)
      {
        auto elemVal = visit_unary(arrEnt + idx, el, op);
        newArr.emplace_back(elemVal);
        ++idx;
      }

      return make_array(m_curEntity, newArr);
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

    auto bitwise_and(detail::generic_type auto lhs, detail::generic_type auto rhs) noexcept
    {
      auto caster = get_caster<int_type>();
      auto intL = caster(lhs);
      auto intR = caster(rhs);
      if (intL && intR)
        return visit_binary(*intL, *intR, [](auto l, auto r) noexcept { return l & r; });

      return get_empty();
    }

    auto bitwise_xor(detail::generic_type auto lhs, detail::generic_type auto rhs) noexcept
    {
      auto caster = get_caster<int_type>();
      auto intL = caster(lhs);
      auto intR = caster(rhs);
      if (intL && intR)
        return visit_binary(*intL, *intR, [](auto l, auto r) noexcept { return l ^ r; });

      return get_empty();
    }

    auto bitwise_or(detail::generic_type auto lhs, detail::generic_type auto rhs) noexcept
    {
      auto caster = get_caster<int_type>();
      auto intL = caster(lhs);
      auto intR = caster(rhs);
      if (intL && intR)
        return visit_binary(*intL, *intR, [](auto l, auto r) noexcept { return l | r; });

      return get_empty();
    }

    // Simple arithmetic

    auto add(detail::addable auto lhs, detail::addable auto rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return l + r;
        });
    }
    auto add(detail::generic_type auto, detail::generic_type auto) noexcept { return get_empty(); }

    auto sub(detail::subtractable auto lhs, detail::subtractable auto rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return l - r;
        });
    }
    auto sub(detail::generic_type auto, detail::generic_type auto) noexcept { return get_empty(); }

    auto mul(detail::multipliable auto lhs, detail::multipliable auto rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return l * r;
        });
    }
    auto mul(detail::generic_type auto, detail::generic_type auto) noexcept { return get_empty(); }

    auto div(detail::divisible auto lhs, detail::divisible auto rhs) noexcept
    {
      if constexpr (is_same_noquals_v<decltype(lhs), int_type>)
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
    auto div(detail::generic_type auto, detail::generic_type auto) noexcept { return get_empty(); }

    // Modulo

    auto mod(detail::fmod_divisible auto lhs, detail::fmod_divisible auto rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return std::fmod(l, r);
        });
    }
    auto mod(detail::modulo_divisible auto lhs, detail::modulo_divisible auto rhs) noexcept
    {
      if constexpr (is_same_noquals_v<decltype(lhs), int_type>)
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
    auto mod(detail::generic_type auto, detail::generic_type auto) noexcept { return get_empty(); }

    // Pow and root

    template <detail::generic_type T> requires (std::is_arithmetic_v<T>)
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
    auto enforce_complex(const detail::generic_type auto&, const detail::generic_type auto&) noexcept { return typed_value<complex_type>{}; }

    auto power(detail::pow_raisable auto base, detail::pow_raisable auto exp) noexcept
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
    auto power(detail::generic_type auto base, detail::generic_type auto exp) noexcept
    {
      auto caster = get_caster<float_type>();
      auto floatL = caster(base);
      auto floatR = caster(exp);
      if (floatL && floatR)
        return power(*floatL, *floatR);

      return get_empty();
    }

    auto root(detail::invertible auto base, detail::invertible auto exp) noexcept
    {
      if constexpr (is_same_noquals_v<decltype(base), int_type>)
        return root(static_cast<float_type>(base), static_cast<float_type>(exp));
      else
        return power(base, eval::inv(exp));
    }
    auto root(detail::generic_type auto, detail::generic_type auto) noexcept { return get_empty(); }

    // Relation and equality

    auto equal(detail::eq_comparable auto lhs, detail::eq_comparable auto rhs, bool compareForEquality) noexcept
    {
      const auto cmp = eval::eq(lhs, rhs);
      const auto res = compareForEquality ? cmp : !cmp;
      return reg_value(res);
    }
    auto equal(detail::generic_type auto, detail::generic_type auto, bool) noexcept { return get_empty(); }

    auto less(detail::rel_comparable auto lhs, detail::rel_comparable auto rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return eval::less(l, r);
        });
    }
    auto less(detail::generic_type auto, detail::generic_type auto) noexcept { return get_empty(); }

    auto less_eq(detail::fully_comparable auto lhs, detail::fully_comparable auto rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return eval::eq(l, r) || eval::less(l, r);
        });
    }
    auto less_eq(detail::generic_type auto, detail::generic_type auto) noexcept { return get_empty(); }

    auto greater(detail::fully_comparable auto lhs, detail::fully_comparable auto rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return !eval::eq(l, r) && !eval::less(l, r);
        });
    }
    auto greater(detail::generic_type auto, detail::generic_type auto) noexcept { return get_empty(); }

    auto greater_eq(detail::fully_comparable auto lhs, detail::fully_comparable auto rhs) noexcept
    {
      return visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return !eval::less(l, r);
        });
    }
    auto greater_eq(detail::generic_type auto, detail::generic_type auto) noexcept { return get_empty(); }

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

    template <>
    value visit_binary(array_type l, array_type r, val_ops op) noexcept
    {
      const auto newSz = l->size() * r->size();
      auto&& newArr = m_registry.allocate_array(m_curEntity, newSz);
      const auto arrEnt = *id_param_t{ &newArr };
      for (auto idx = size_type{}; auto el : *l)
      {
        for (auto er : *r)
        {
          const auto valEnt = arrEnt + idx;
          auto newVal = visit_binary(valEnt, el, er, op);
          newArr.emplace_back(newVal);
          ++idx;
        }
        ++idx;
      }

      return make_array(m_curEntity, newArr);
    }

    template <detail::generic_type T>
    arr_t to_unit_array(const T& val) noexcept
    {
      registry::val_array arr;
      eval::value elem;
      if constexpr (!is_same_noquals_v<T, invalid_val_t>)
        elem = { &val };
      
      arr.emplace_back(elem);
      return arr;
    }

    template <detail::generic_type T>
      requires (!is_same_noquals_v<T, array_type>)
    value visit_binary(array_type l, T r, val_ops op) noexcept
    {
      auto arr = to_unit_array(r);
      auto res = visit_binary(l, array_type{ arr }, op);
      return res;
    }

    template <detail::generic_type T>
      requires (!is_same_noquals_v<T, array_type>)
    value visit_binary(T l, array_type r, val_ops op) noexcept
    {
      auto arr = to_unit_array(l);
      auto res = visit_binary(array_type{ arr }, r, op);
      return res;
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
    value reg_value(detail::expr_result auto val) noexcept
    {
      if (m_curEntity != invalidEnt)
      {
        return m_registry.register_entity(m_curEntity, std::move(val));
      }

      return m_registry.register_entity({}, val);
    }

    //
    // Registers result of assignment operations
    //
    value visit_assign(detail::generic_type auto rhs) noexcept
    {
      return reg_value(std::move(rhs));
    }

    //
    // Intermediate binary visitor
    // Dispatches the right operand according to its type
    //
    value visit_binary(detail::generic_type auto lhs, value rhs, val_ops op) noexcept
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
    // Retrieves an array object allocated for the specified entity
    //
    arr_t& new_array(id_param_t ent, size_type size) noexcept
    {
      return m_registry.allocate_array(*ent, size);
    }

    //
    // Makes an array instance based on the underlying data
    //
    value make_array(id_param_t ent, arr_t& data) noexcept
    {
      value_guard _{ m_curEntity, *ent };
      return reg_value(array_type{ data });
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