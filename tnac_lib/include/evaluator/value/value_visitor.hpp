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

      ent_id(id_t id) noexcept         : value{ id } {}
      ent_id(const void* ent) noexcept : ent_id{ reinterpret_cast<id_t>(ent) } {}
      ent_id(std::nullptr_t) noexcept  : ent_id{} {}

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
    void visit_unary(T val, F&& op) noexcept
    {
      reg_value(op(std::move(val)));
    }

    void unary_plus(detail::plusable auto operand) noexcept
    {
      visit_unary(std::move(operand), [](auto val) noexcept { return +val; });
    }
    void unary_plus(detail::generic_type auto) noexcept { clear_result(); }

    void unary_neg(detail::negatable auto operand) noexcept
    {
      visit_unary(operand, [](auto val) noexcept { return -val; });
    }
    void unary_neg(detail::generic_type auto) noexcept { clear_result(); }

    void bitwise_not(detail::generic_type auto operand) noexcept
    {
      if (auto intOp = get_caster<int_type>()(std::move(operand)))
      {
        visit_unary(*intOp, [](auto val) noexcept { return ~val; });
        return;
      }

      clear_result();
    }

    void logical_not(detail::generic_type auto operand) noexcept
    {
      auto boolOp = get_caster<bool_type>()(std::move(operand));
      visit_unary(boolOp.value_or(false), [](auto val) noexcept { return !val; });
    }

    void logical_is(detail::generic_type auto operand) noexcept
    {
      auto boolOp = get_caster<bool_type>()(std::move(operand));
      visit_unary(boolOp && *boolOp, [](auto val) noexcept { return val; });
    }

    void abs(detail::abs_compatible auto operand) noexcept
    {
      visit_unary(std::move(operand), [](auto val) noexcept
        {
          return eval::abs(val);
        });
    }
    void abs(detail::generic_type auto) noexcept { clear_result(); }

    //
    // Dispatches unary operations according to operator type
    //
    template <detail::generic_type T>
    void visit_unary(T operand, val_ops op) noexcept
    {
      using op_type = common_type_t<T, T>;
      auto val = get_caster<op_type>()(std::move(operand));
      if (!val)
      {
        clear_result();
        return;
      }

      using enum val_ops;
      switch (op)
      {
      case UnaryNegation:   unary_neg(std::move(*val));   break;
      case UnaryPlus:       unary_plus(std::move(*val));  break;
      case UnaryBitwiseNot: bitwise_not(std::move(*val)); break;
      case LogicalNot:      logical_not(std::move(*val)); break;
      case LogicalIs:       logical_is(std::move(*val));  break;
      case AbsoluteValue:   abs(std::move(*val));         break;

      default: clear_result(); break;
      }
    }

    template <>
    void visit_unary(array_type operand, val_ops op) noexcept
    {
      utils::unused(operand, op);
      //if (utils::eq_any(op, val_ops::LogicalIs, val_ops::LogicalNot))
      //{
      //  auto toBool = get_caster<bool_type>()(std::move(operand));
      //  return visit_unary(toBool.value_or(false), op);
      //}

      //auto&& newArr = m_registry.allocate_array(m_curEntity, operand->size());
      //for (auto el : *operand)
      //{
      //  auto&& elemVal = newArr.emplace_back();
      //  elemVal = visit_unary(&elemVal, el, op);
      //}

      //return make_array(m_curEntity, newArr);
    }

  private: // Binary operations

    //
    // Registers result of binary operations
    //
    template <detail::generic_type L, detail::generic_type R, detail::binary_function<L, R> F>
    void visit_binary(L lhs, R rhs, F&& op) noexcept
    {
      reg_value(op(std::move(lhs), std::move(rhs)));
    }

    // Bitwise

    void bitwise_and(detail::generic_type auto lhs, detail::generic_type auto rhs) noexcept
    {
      auto caster = get_caster<int_type>();
      auto intL = caster(lhs);
      auto intR = caster(rhs);
      if (intL && intR)
      {
        visit_binary(*intL, *intR, [](auto l, auto r) noexcept { return l & r; });
        return;
      }

      clear_result();
    }

    void bitwise_xor(detail::generic_type auto lhs, detail::generic_type auto rhs) noexcept
    {
      auto caster = get_caster<int_type>();
      auto intL = caster(lhs);
      auto intR = caster(rhs);
      if (intL && intR)
      {
        visit_binary(*intL, *intR, [](auto l, auto r) noexcept { return l ^ r; });
        return;
      }

      clear_result();
    }

    void bitwise_or(detail::generic_type auto lhs, detail::generic_type auto rhs) noexcept
    {
      auto caster = get_caster<int_type>();
      auto intL = caster(lhs);
      auto intR = caster(rhs);
      if (intL && intR)
      {
        visit_binary(*intL, *intR, [](auto l, auto r) noexcept { return l | r; });
        return;
      }

      clear_result();
    }

    // Simple arithmetic

    void add(detail::addable auto lhs, detail::addable auto rhs) noexcept
    {
      visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return l + r;
        });
    }
    void add(detail::generic_type auto, detail::generic_type auto) noexcept { clear_result(); }

    void sub(detail::subtractable auto lhs, detail::subtractable auto rhs) noexcept
    {
      visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return l - r;
        });
    }
    void sub(detail::generic_type auto, detail::generic_type auto) noexcept { clear_result(); }

    void mul(detail::multipliable auto lhs, detail::multipliable auto rhs) noexcept
    {
      visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return l * r;
        });
    }
    void mul(detail::generic_type auto, detail::generic_type auto) noexcept { clear_result(); }

    void div(detail::divisible auto lhs, detail::divisible auto rhs) noexcept
    {
      if constexpr (is_same_noquals_v<decltype(lhs), int_type>)
      {
        div(static_cast<float_type>(lhs), static_cast<float_type>(rhs));
      }
      else
      {
        visit_binary(std::move(lhs), std::move(rhs),
          [](auto l, auto r) noexcept
          {
            return l / r;
          });
      }
    }
    void div(detail::generic_type auto, detail::generic_type auto) noexcept { clear_result(); }

    // Modulo

    void mod(detail::fmod_divisible auto lhs, detail::fmod_divisible auto rhs) noexcept
    {
      visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return std::fmod(l, r);
        });
    }
    void mod(detail::modulo_divisible auto lhs, detail::modulo_divisible auto rhs) noexcept
    {
      if constexpr (is_same_noquals_v<decltype(lhs), int_type>)
      {
        mod(static_cast<float_type>(lhs), static_cast<float_type>(rhs));
      }
      else
      {
        visit_binary(std::move(lhs), std::move(rhs),
          [](auto l, auto r) noexcept
          {
            return l % r;
          });
      }
    }
    void mod(detail::generic_type auto, detail::generic_type auto) noexcept { clear_result(); }

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

    void power(detail::pow_raisable auto base, detail::pow_raisable auto exp) noexcept
    {
      if (auto cpl = enforce_complex(base, exp))
      {
        reg_value(*cpl);
        return;
      }

      visit_binary(std::move(base), std::move(exp),
        [](auto l, auto r) noexcept
        {
          return std::pow(l, r);
        });
    }
    void power(detail::generic_type auto base, detail::generic_type auto exp) noexcept
    {
      auto caster = get_caster<float_type>();
      auto floatL = caster(base);
      auto floatR = caster(exp);
      if (floatL && floatR)
      {
        power(*floatL, *floatR);
        return;
      }

      clear_result();
    }

    void root(detail::invertible auto base, detail::invertible auto exp) noexcept
    {
      if constexpr (is_same_noquals_v<decltype(base), int_type>)
        root(static_cast<float_type>(base), static_cast<float_type>(exp));
      else
        power(base, eval::inv(exp));
    }
    void root(detail::generic_type auto, detail::generic_type auto) noexcept { clear_result(); }

    // Relation and equality

    void equal(detail::eq_comparable auto lhs, detail::eq_comparable auto rhs, bool compareForEquality) noexcept
    {
      const auto cmp = eval::eq(lhs, rhs);
      const auto res = compareForEquality ? cmp : !cmp;
      reg_value(res);
    }
    void equal(detail::generic_type auto, detail::generic_type auto, bool) noexcept { clear_result(); }

    void less(detail::rel_comparable auto lhs, detail::rel_comparable auto rhs) noexcept
    {
      visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return eval::less(l, r);
        });
    }
    void less(detail::generic_type auto, detail::generic_type auto) noexcept { clear_result(); }

    void less_eq(detail::fully_comparable auto lhs, detail::fully_comparable auto rhs) noexcept
    {
      visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return eval::eq(l, r) || eval::less(l, r);
        });
    }
    void less_eq(detail::generic_type auto, detail::generic_type auto) noexcept { clear_result(); }

    void greater(detail::fully_comparable auto lhs, detail::fully_comparable auto rhs) noexcept
    {
      visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return !eval::eq(l, r) && !eval::less(l, r);
        });
    }
    void greater(detail::generic_type auto, detail::generic_type auto) noexcept { clear_result(); }

    void greater_eq(detail::fully_comparable auto lhs, detail::fully_comparable auto rhs) noexcept
    {
      visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return !eval::less(l, r);
        });
    }
    void greater_eq(detail::generic_type auto, detail::generic_type auto) noexcept { clear_result(); }

    //
    // Dispatches binary operations according to operator type
    //
    template <detail::generic_type L, detail::generic_type R>
    void visit_binary(L l, R r, val_ops op) noexcept
    {
      using common_t = common_type_t<L, R>;
      auto caster = get_caster<common_t>();
      auto lhs = caster(std::move(l));
      auto rhs = caster(std::move(r));
      if (!lhs || !rhs)
      {
        clear_result();
        return;
      }

      using enum val_ops;
      switch (op)
      {
      case Addition:       add(std::move(*lhs), std::move(*rhs)); break;
      case Subtraction:    sub(std::move(*lhs), std::move(*rhs)); break;
      case Multiplication: mul(std::move(*lhs), std::move(*rhs)); break;
      case Division:       div(std::move(*lhs), std::move(*rhs)); break;
      case Modulo:         mod(std::move(*lhs), std::move(*rhs)); break;

      case RelLess:   less(std::move(*lhs), std::move(*rhs));         break;
      case RelLessEq: less_eq(std::move(*lhs), std::move(*rhs));      break;
      case RelGr:     greater(std::move(*lhs), std::move(*rhs));      break;
      case RelGrEq:   greater_eq(std::move(*lhs), std::move(*rhs));   break;
      case Equal:     equal(std::move(*lhs), std::move(*rhs), true);  break;
      case NEqual:    equal(std::move(*lhs), std::move(*rhs), false); break;

      case BitwiseAnd: bitwise_and(std::move(*lhs), std::move(*rhs)); break;
      case BitwiseXor: bitwise_xor(std::move(*lhs), std::move(*rhs)); break;
      case BitwiseOr:  bitwise_or(std::move(*lhs), std::move(*rhs));  break;

      case BinaryPow:  power(std::move(*lhs), std::move(*rhs)); break;
      case BinaryRoot: root(std::move(*lhs), std::move(*rhs));  break;

      default: clear_result(); break;
      }
    }

    template <>
    void visit_binary(array_type l, array_type r, val_ops op) noexcept
    {
      utils::unused(l, r, op);
      //const auto newSz = l->size() * r->size();
      //auto&& newArr = m_registry.allocate_array(m_curEntity, newSz);
      //for (auto el : *l)
      //{
      //  for (auto er : *r)
      //  {
      //    auto&& newVal = newArr.emplace_back();
      //    newVal = visit_binary(&newVal, el, er, op);
      //  }
      //}

      //return make_array(m_curEntity, newArr);
    }

    template <detail::generic_type T>
    arr_t to_unit_array(const T& val) noexcept
    {
      utils::unused(val); return {};
      //registry::val_array arr;
      //eval::value elem;
      //if constexpr (!is_same_noquals_v<T, invalid_val_t>)
      //  elem = { &val };
      //
      //arr.emplace_back(elem);
      //return arr;
    }

    template <detail::generic_type T>
      requires (!is_same_noquals_v<T, array_type>)
    void visit_binary(array_type l, T r, val_ops op) noexcept
    {
      utils::unused(l, r, op);
      //auto arr = to_unit_array(r);
      //auto res = visit_binary(l, array_type{ arr }, op);
      //return res;
    }

    template <detail::generic_type T>
      requires (!is_same_noquals_v<T, array_type>)
    void visit_binary(T l, array_type r, val_ops op) noexcept
    {
      utils::unused(l, r, op);
      //auto arr = to_unit_array(l);
      //auto res = visit_binary(array_type{ arr }, r, op);
      //return res;
    }

  private:
    //
    // Extracts type from value and calls the specified function
    //
    template <typename F>
    void visit_value(value val, F&& func) noexcept
    {
      on_value(val, std::forward<F>(func));
    }

    //
    // Registers the value in the registry
    //
    void reg_value(detail::generic_type auto val) noexcept
    {
      if (m_curEntity != invalidEnt)
      {
        m_registry.register_entity(m_curEntity, std::move(val));
        return;
      }

      m_registry.push(std::move(val));
    }

    //
    // Registers result of assignment operations
    //
    void visit_assign(detail::generic_type auto rhs) noexcept
    {
      reg_value(std::move(rhs));
    }

    //
    // Intermediate binary visitor
    // Dispatches the right operand according to its type
    //
    void visit_binary(detail::generic_type auto lhs, value rhs, val_ops op) noexcept
    {
      visit_value(rhs, [this, l = std::move(lhs), op](auto rhs) noexcept
        {
          visit_binary(std::move(l), std::move(rhs), op);
        });
    }

    //
    // Dispatches the instantiation call
    //
    template <detail::expr_result Obj, typename T, T... Seq>
    void instantiate(const std::array<value, sizeof...(Seq)>& args, std::integer_sequence<T, Seq...>) noexcept
    {
      using type_info = eval::type_info<Obj>;
      using type_gen = type_wrapper<Obj>;
      auto instance = type_gen{}(cast_value<utils::id_to_type_t<type_info::params[Seq]>>(args[Seq])...);

      if (!instance)
      {
        clear_result();
        return;
      }

      reg_value(std::move(*instance));
    }

  public:
    //
    // Instantiates an object
    //
    template <detail::expr_result Obj, typename... Args>
      requires is_all_v<value, Args...>
    void instantiate(Args ...args) noexcept
    {
      using type_info = eval::type_info<Obj>;
      static constexpr auto max = type_info::maxArgs;
      static_assert(sizeof ...(Args) == max);

      const std::array argList{ args... };
      instantiate<Obj>(argList, std::make_index_sequence<max>{});
    }

    //
    // Stores a value for a function
    //
    value make_function(id_param_t ent, function_type f) noexcept
    {
      value_guard _{ m_curEntity, *ent };
      reg_value(f);
      return {};
    }

    //
    // Evaluates a binary expr
    //
    void visit_binary(value lhs, value rhs, val_ops op) noexcept
    {
      if (!detail::is_binary(op))
        clear_result();

      visit_value(lhs, [this, rhs, op](auto lhs) noexcept
        {
          visit_binary(std::move(lhs), std::move(rhs), op);
        });
    }

    //
    // Evaluates a unary expr
    //
    void visit_unary(value val, val_ops op) noexcept
    {
      if (!detail::is_unary(op))
      {
        clear_result();
        return;
      }

      visit_value(val, [this, op](auto v) noexcept
        {
          visit_unary(std::move(v), op);
        });
    }

    //
    // Pushes the given value to the end of the queue
    //
    void push_value(value val) noexcept
    {
      visit_value(val, [this](auto v) noexcept
        {
          reg_value(std::move(v));
        });
    }

    //
    // Makes a value for an assigned-to entity
    //
    value visit_assign(id_param_t ent, value rhs) noexcept
    {
      value_guard _{ m_curEntity, *ent };

      visit_value(rhs, [this](auto v) noexcept
        {
          visit_assign(std::move(v));
        });

      return {};
    }

    //
    // Parses an integer literal value from string
    //
    void visit_int_literal(string_t src, int base) noexcept
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
      {
        clear_result();
        return;
      }

      reg_value(result);
    }

    //
    // Parses a floating point literal value from string
    //
    void visit_float_literal(string_t src) noexcept
    {
      auto begin = src.data();
      auto end = begin + src.length();

      float_type result{};
      auto convRes = std::from_chars(begin, end, result);
      if (convRes.ec != std::errc{ 0 })
      {
        clear_result();
        return;
      }

      reg_value(result);
    }

    //
    // Registers a boolean literal
    //
    void visit_bool_literal(bool value) noexcept
    {
      return reg_value(value);
    }

    //
    // Makes an array instance based on the underlying data
    //
    void make_array(arr_t& data) noexcept
    {
      reg_value(array_type{ data });
    }

    //
    // Retrieves the next value from the queue of temporaries
    //
    temporary fetch_next() noexcept
    {
      return m_registry.consume();
    }

    //
    // Resets the last evaluation result and returns an empty value
    //
    void clear_result() noexcept
    {
      reg_value(eval::invalid_val_t{});
    }


  private:
    static constexpr auto invalidEnt = detail::ent_id::invalid_id();
    entity_id m_curEntity{ invalidEnt };
    registry& m_registry;
  };
}