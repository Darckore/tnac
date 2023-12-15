//
// Value visitor
//

#pragma once
#include "eval/value/value_registry.hpp"
#include "eval/types/traits.hpp"

namespace tnac::eval::detail
{
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
  constexpr auto is_comparison(val_ops op) noexcept
  {
    using enum val_ops;
    return utils::eq_any(op, RelLess, RelLessEq, RelGr, RelGrEq, Equal, NEqual);
  }

  template <typename T>
  concept value_container = requires (T & t)
  {
    typename T::value_type;
    typename T::size_type;
      requires std::same_as<typename T::value_type, stored_value>;
    { t[typename T::size_type{}] } -> std::same_as<stored_value&>;
  };
}

namespace tnac::eval
{
  //
  // Value visitor used in expression evaluations
  //
  class value_visitor final
  {
  public:
    using enum val_ops;
    using arr_t      = registry::val_array;
    using size_type  = registry::size_type;

  public:
    CLASS_SPECIALS_NONE(value_visitor);

    ~value_visitor() noexcept;
    explicit value_visitor(registry& reg) noexcept;

  private: // Unary operations
    //
    // Registers result of unary operations
    //
    template <detail::expr_result T, detail::unary_function<T> F>
    void visit_unary(T val, F&& op) noexcept
    {
      reg_value(op(std::move(val)));
    }

    void unary_plus(detail::plusable auto operand) noexcept
    {
      visit_unary(std::move(operand), [](auto val) noexcept { return +val; });
    }
    void unary_plus(detail::expr_result auto) noexcept { clear_result(); }

    void unary_neg(detail::negatable auto operand) noexcept
    {
      visit_unary(operand, [](auto val) noexcept { return -val; });
    }
    void unary_neg(detail::expr_result auto) noexcept { clear_result(); }

    void bitwise_not(detail::expr_result auto operand) noexcept
    {
      if (auto intOp = get_caster<int_type>()(std::move(operand)))
      {
        visit_unary(*intOp, [](auto val) noexcept { return ~val; });
        return;
      }

      clear_result();
    }

    void logical_not(detail::expr_result auto operand) noexcept
    {
      auto boolOp = get_caster<bool_type>()(std::move(operand));
      visit_unary(boolOp.value_or(false), [](auto val) noexcept { return !val; });
    }

    void logical_is(detail::expr_result auto operand) noexcept
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
    void abs(detail::expr_result auto) noexcept { clear_result(); }

    //
    // Dispatches unary operations according to operator type
    //
    template <detail::expr_result T>
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
      if (utils::eq_any(op, val_ops::LogicalIs, val_ops::LogicalNot))
      {
        auto toBool = get_caster<bool_type>()(std::move(operand));
        return visit_unary(toBool.value_or(false), op);
      }

      value_lock _{ operand, m_registry };
      for (auto&& el : *operand)
      {
        visit_unary(*el, op);
      }

      make_array(operand->size());
    }

  private: // Binary operations
    //
    // Registers result of binary operations
    //
    template <detail::expr_result L, detail::expr_result R, detail::binary_function<L, R> F>
    void visit_binary(L lhs, R rhs, F&& op) noexcept
    {
      reg_value(op(std::move(lhs), std::move(rhs)));
    }

    // Bitwise

    void bitwise_and(detail::expr_result auto lhs, detail::expr_result auto rhs) noexcept
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

    void bitwise_xor(detail::expr_result auto lhs, detail::expr_result auto rhs) noexcept
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

    void bitwise_or(detail::expr_result auto lhs, detail::expr_result auto rhs) noexcept
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
    void add(detail::expr_result auto, detail::expr_result auto) noexcept { clear_result(); }

    void sub(detail::subtractable auto lhs, detail::subtractable auto rhs) noexcept
    {
      visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return l - r;
        });
    }
    void sub(detail::expr_result auto, detail::expr_result auto) noexcept { clear_result(); }

    void mul(detail::multipliable auto lhs, detail::multipliable auto rhs) noexcept
    {
      visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return l * r;
        });
    }
    void mul(detail::expr_result auto, detail::expr_result auto) noexcept { clear_result(); }

    void div(detail::divisible auto lhs, detail::divisible auto rhs) noexcept
    {
      if constexpr (utils::same_noquals<decltype(lhs), int_type>)
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
    void div(detail::expr_result auto, detail::expr_result auto) noexcept { clear_result(); }

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
      if constexpr (utils::same_noquals<decltype(lhs), int_type>)
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
    void mod(detail::expr_result auto, detail::expr_result auto) noexcept { clear_result(); }

    // Pow and root

    template <detail::expr_result T> requires (std::is_arithmetic_v<T>)
    auto enforce_complex(const T& l, const T& r) noexcept -> typed_value<complex_type>
    {
      auto base = static_cast<float_type>(l);
      if (base > 0.0 || utils::eq(base, 0.0))
        return {};

      auto exp = static_cast<float_type>(r);
      const auto root = utils::inv(exp);
      const auto square = 2.0;
      if (const auto mod2 = std::fmod(root, square); !utils::eq(mod2, 0.0))
        return {};

      const auto remainder = utils::inv(root / square);
      const auto res   = complex_type{ 0.0, std::pow(utils::abs(base), utils::inv(square)) };
      const auto intrm = utils::eq(utils::abs(remainder), 1.0) ? res : std::pow(res, remainder);
      return (remainder > 0.0) ? intrm : eval::inv(intrm);
    }
    auto enforce_complex(const detail::expr_result auto&, const detail::expr_result auto&) noexcept { return typed_value<complex_type>{}; }

    template <detail::expr_result T> requires (std::is_arithmetic_v<T>)
    auto neg_root(const T& l, const T& r) noexcept -> typed_value<float_type>
    {
      auto base = static_cast<float_type>(l);
      if (base > 0.0 || utils::eq(base, 0.0))
        return {};

      auto exp = static_cast<float_type>(r);
      if (const auto mod2 = std::fmod(utils::inv(exp), 2.0); utils::eq(mod2, 0.0))
        return {};

      return -std::pow(utils::abs(base), exp);
    }
    auto neg_root(const detail::expr_result auto&, const detail::expr_result auto&) noexcept { return typed_value<float_type>{}; }

    void power(detail::pow_raisable auto base, detail::pow_raisable auto exp) noexcept
    {
      if (auto cpl = enforce_complex(base, exp))
      {
        reg_value(*cpl);
        return;
      }
      else if (auto neg = neg_root(base, exp))
      {
        reg_value(*neg);
        return;
      }

      visit_binary(std::move(base), std::move(exp),
        [](auto l, auto r) noexcept
        {
          return std::pow(l, r);
        });
    }
    void power(detail::expr_result auto base, detail::expr_result auto exp) noexcept
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
      if constexpr (utils::same_noquals<decltype(base), int_type>)
        root(static_cast<float_type>(base), static_cast<float_type>(exp));
      else
        power(base, eval::inv(exp));
    }
    void root(detail::expr_result auto, detail::expr_result auto) noexcept { clear_result(); }

    // Relation and equality

    void equal(detail::eq_comparable auto lhs, detail::eq_comparable auto rhs, bool compareForEquality) noexcept
    {
      const auto cmp = eval::eq(lhs, rhs);
      const auto res = compareForEquality ? cmp : !cmp;
      reg_value(res);
    }
    void equal(array_type lhs, array_type rhs, bool compareForEquality) noexcept
    {
      auto&& l = *lhs;
      auto&& r = *rhs;

      if (compareForEquality && &l == &r)
      {
        reg_value(true);
        return;
      }
      if (l.size() != r.size())
      {
        reg_value(!compareForEquality);
        return;
      }

      for (auto&& [le, re] : utils::make_iterators(l, r))
      {
        visit_binary(*le, *re, val_ops::Equal);
        const auto cmp = to_bool(*fetch_next());
        if (!cmp)
        {
          reg_value(compareForEquality ? false : true);
          return;
        }
      }

      reg_value(compareForEquality);
    }
    void equal(detail::expr_result auto, detail::expr_result auto, bool) noexcept { clear_result(); }

    void less(detail::rel_comparable auto lhs, detail::rel_comparable auto rhs) noexcept
    {
      visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return eval::less(l, r);
        });
    }
    void less(array_type lhs, array_type rhs) noexcept
    {
      auto&& l = *lhs;
      auto&& r = *rhs;
      if (&l == &r)
      {
        reg_value(false);
        return;
      }

      for (auto&& [lv, rv] : utils::make_iterators(l, r))
      {
        visit_binary(*lv, *rv, val_ops::RelLess);
        const auto cmp = to_bool(*fetch_next());
        if (cmp)
        {
          reg_value(true);
          return;
        }
      }

      reg_value(l.size() < r.size());
    }
    void less(detail::expr_result auto, detail::expr_result auto) noexcept { clear_result(); }

    void less_eq(detail::fully_comparable auto lhs, detail::fully_comparable auto rhs) noexcept
    {
      visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return eval::eq(l, r) || eval::less(l, r);
        });
    }
    void less_eq(array_type lhs, array_type rhs) noexcept
    {
      less(lhs, rhs);
      if (to_bool(*fetch_next()))
      {
        reg_value(true);
        return;
      }
      equal(std::move(lhs), std::move(rhs), true);
      reg_value(to_bool(*fetch_next()));
    }
    void less_eq(detail::expr_result auto, detail::expr_result auto) noexcept { clear_result(); }

    void greater(detail::fully_comparable auto lhs, detail::fully_comparable auto rhs) noexcept
    {
      visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return !eval::eq(l, r) && !eval::less(l, r);
        });
    }
    void greater(array_type lhs, array_type rhs) noexcept
    {
      auto&& l = *lhs;
      auto&& r = *rhs;
      if (&l == &r)
      {
        reg_value(false);
        return;
      }

      for (auto&& [lv, rv] : utils::make_iterators(l, r))
      {
        visit_binary(*lv, *rv, val_ops::RelGr);
        const auto cmp = to_bool(*fetch_next());
        if (cmp)
        {
          reg_value(true);
          return;
        }
      }

      reg_value(l.size() > r.size());
    }
    void greater(detail::expr_result auto, detail::expr_result auto) noexcept { clear_result(); }

    void greater_eq(detail::fully_comparable auto lhs, detail::fully_comparable auto rhs) noexcept
    {
      visit_binary(std::move(lhs), std::move(rhs),
        [](auto l, auto r) noexcept
        {
          return !eval::less(l, r);
        });
    }
    void greater_eq(array_type lhs, array_type rhs) noexcept
    {
      greater(lhs, rhs);
      if (to_bool(*fetch_next()))
      {
        reg_value(true);
        return;
      }
      equal(std::move(lhs), std::move(rhs), true);
      reg_value(to_bool(*fetch_next()));
    }
    void greater_eq(detail::expr_result auto, detail::expr_result auto) noexcept { clear_result(); }

    //
    // Dispatches binary operations according to operator type
    //
    template <detail::expr_result L, detail::expr_result R>
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

    template <detail::expr_result T>
    auto to_unit_array(const T& val) noexcept
    {
      arr_t arr;
      arr.emplace_back(val);
      return arr;
    }

    template <>
    void visit_binary(array_type l, array_type r, val_ops op) noexcept
    {
      if (detail::is_comparison(op))
      {
        switch (op)
        {
        case RelLess:   less(std::move(l), std::move(r));         break;
        case RelLessEq: less_eq(std::move(l), std::move(r));      break;
        case RelGr:     greater(std::move(l), std::move(r));      break;
        case RelGrEq:   greater_eq(std::move(l), std::move(r));   break;
        case Equal:     equal(std::move(l), std::move(r), true);  break;
        case NEqual:    equal(std::move(l), std::move(r), false); break;
        }
        return;
      }

      static auto unitArr = []() noexcept { arr_t ret; ret.emplace_back(int_type{}); return ret; }();
      const auto lsz = l->size();
      if (!lsz) l = array_type{ unitArr };
      const auto rsz = r->size();
      if (!rsz) r = array_type{ unitArr };
      const auto newSz = (lsz && rsz) ? lsz * rsz : std::max(lsz, rsz);

      value_lock _l{ l, m_registry };
      value_lock _r{ r, m_registry };

      for (auto&& el : *l)
      {
        for (auto&& er : *r)
        {
          visit_binary(*el, *er, op);
        }
      }

      make_array(newSz);
    }

    template <detail::expr_result T>
      requires (!utils::same_noquals<T, array_type>)
    void visit_binary(array_type l, T r, val_ops op) noexcept
    {
      auto rhs = to_unit_array(r);
      visit_binary(std::move(l), array_type{ rhs }, op);
    }

    template <detail::expr_result T>
      requires (!utils::same_noquals<T, array_type>)
    void visit_binary(T l, array_type r, val_ops op) noexcept
    {
      auto lhs = to_unit_array(l);
      visit_binary(array_type{ lhs }, std::move(r), op);
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
    void reg_value(detail::expr_result auto val) noexcept
    {
      if (m_curEntity != entity_id{})
      {
        m_registry.register_entity(m_curEntity, std::move(val));
        return;
      }

      m_registry.push(std::move(val));
    }

    //
    // Registers result of assignment operations
    //
    void visit_assign(detail::expr_result auto rhs) noexcept
    {
      reg_value(std::move(rhs));
    }

    //
    // Intermediate binary visitor
    // Dispatches the right operand according to its type
    //
    void visit_binary(detail::expr_result auto lhs, value rhs, val_ops op) noexcept
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
    void instantiate(const std::array<stored_value, sizeof...(Seq)>& args, std::integer_sequence<T, Seq...>) noexcept
    {
      using type_info = eval::type_info<Obj>;
      using type_gen = type_wrapper<Obj>;
      auto instance = type_gen{}(cast_value<utils::id_to_type_t<type_info::params[Seq]>>(*args[Seq])...);

      if (!instance)
      {
        clear_result();
        return;
      }

      reg_value(std::move(*instance));
    }

  public:
    //
    // Locks the given ref counted value
    //
    auto lock(const detail::lockable auto& val) noexcept
    {
      return value_lock{ val, m_registry };
    }

    //
    // Instantiates an object
    //
    template <detail::expr_result Obj, typename... Args>
      requires utils::all_same<stored_value, Args...>
    void instantiate(Args ...args) noexcept
    {
      using type_info = eval::type_info<Obj>;
      static constexpr auto max = type_info::maxArgs;
      static_assert(sizeof ...(Args) == max);

      const std::array argList{ std::move(args)... };
      instantiate<Obj>(argList, std::make_index_sequence<max>{});
    }

    //
    // Stores a value for a function
    //
    value make_function(entity_id ent, function_type f) noexcept;

    //
    // Evaluates a binary expr
    //
    void visit_binary(value lhs, value rhs, val_ops op) noexcept;

    //
    // Evaluates a unary expr
    //
    void visit_unary(value val, val_ops op) noexcept;

    //
    // Pushes the given value to the stack
    //
    void push_value(value val) noexcept;

    //
    // Pushes the last result to the stack
    //
    void push_last() noexcept;

    //
    // Makes a value for an assigned-to entity
    //
    value visit_assign(entity_id ent, value rhs) noexcept;

    //
    // Parses an integer literal value from string
    //
    void visit_int_literal(string_t src, int base) noexcept;

    //
    // Parses a floating point literal value from string
    //
    void visit_float_literal(string_t src) noexcept;

    //
    // Registers a boolean literal
    //
    void visit_bool_literal(bool value) noexcept;

    //
    // Registers an imaginary 'i' value
    //
    void visit_i() noexcept;

    //
    // Registers the value of pi
    //
    void visit_pi() noexcept;

    //
    // Registers the value of pi
    //
    void visit_e() noexcept;

    //
    // Retrieves the next value from the stack of temporaries
    //
    stored_value fetch_next() noexcept;

    //
    // Retrieves the last evaluation result
    //
    value last() noexcept;

    //
    // Resets the last evaluation result and returns an empty value
    //
    void clear_result() noexcept;

    //
    // Fills the specified number of arg values in the given container
    //
    void fill_args(detail::value_container auto& args, size_type count) noexcept
    {
      for (auto idx = count; idx > size_type{}; --idx)
      {
        args[idx - 1] = fetch_next();
      }
    }

    //
    // Creates and registers an array
    //
    void make_array(size_type count) noexcept;

    //
    // Creates a temporary local array and returns it without pushing it into
    // the registry. The result is pre-filled with args from the current stack
    // This is required for call expressions
    //
    arr_t collect_args_locally(size_type count) noexcept;

  private:
    entity_id m_curEntity{};
    registry& m_registry;
  };
}