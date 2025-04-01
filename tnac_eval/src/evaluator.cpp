#if 0
#include "eval/value/evaluator.hpp"

namespace tnac::eval::detail
{
  namespace
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
  }
}

namespace tnac::eval // Internals
{
  void evaluator::reg_value(expr_result auto val) noexcept
  {
    if (m_curEntity != entity_id{})
    {
      m_registry.register_entity(m_curEntity, std::move(val));
      return;
    }

    m_registry.push(std::move(val));
  }

  template <typename F>
  void evaluator::visit_value(value val, F&& func) noexcept
  {
    on_value(val, std::forward<F>(func));
  }

  void evaluator::visit_assign(expr_result auto rhs) noexcept
  {
    reg_value(std::move(rhs));
  }

  template <expr_result T>
  auto evaluator::to_unit_array(const T& val) noexcept
  {
    arr_t arr;
    arr.emplace_back(val);
    return arr;
  }

  void evaluator::fill_args(value_container auto& args, size_type count) noexcept
  {
    for (auto idx = count; idx > size_type{}; --idx)
    {
      args[idx - 1] = fetch_next();
    }
  }

  template <expr_result Obj, typename T, T... Seq>
  void evaluator::instantiate(const std::array<stored_value, sizeof...(Seq)>& args, std::integer_sequence<T, Seq...>) noexcept
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

  template <expr_result Obj>
  void evaluator::instantiate(size_type argSz) noexcept
  {
    static constexpr auto max = type_info<Obj>::maxArgs;
    std::array<stored_value, max> args{};
    fill_args(args, argSz);
    instantiate<Obj>(args, std::make_index_sequence<max>{});
  }
}


namespace tnac::eval // Unary ops
{
  void evaluator::unary_plus(plusable auto operand) noexcept
  {
    visit_unary(std::move(operand), [](auto val) noexcept { return +val; });
  }
  void evaluator::unary_plus(expr_result auto) noexcept
  {
    clear_result();
  }

  void evaluator::unary_neg(negatable auto operand) noexcept
  {
    visit_unary(operand, [](auto val) noexcept { return -val; });
  }
  void evaluator::unary_neg(expr_result auto) noexcept
  {
    clear_result();
  }

  void evaluator::bitwise_not(expr_result auto operand) noexcept
  {
    if (auto intOp = get_caster<int_type>()(std::move(operand)))
    {
      visit_unary(*intOp, [](auto val) noexcept { return ~val; });
      return;
    }

    clear_result();
  }

  void evaluator::logical_not(expr_result auto operand) noexcept
  {
    auto boolOp = get_caster<bool_type>()(std::move(operand));
    visit_unary(boolOp.value_or(false), [](auto val) noexcept { return !val; });
  }

  void evaluator::logical_is(expr_result auto operand) noexcept
  {
    auto boolOp = get_caster<bool_type>()(std::move(operand));
    visit_unary(boolOp && *boolOp, [](auto val) noexcept { return val; });
  }

  void evaluator::abs(abs_compatible auto operand) noexcept
  {
    visit_unary(std::move(operand), [](auto val) noexcept
      {
        return eval::abs(val);
      });
  }
  void evaluator::abs(expr_result auto) noexcept
  {
    clear_result();
  }
}

namespace tnac::eval // Binary ops
{
  // Bitwise

  void evaluator::bitwise_and(expr_result auto lhs, expr_result auto rhs) noexcept
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

  void evaluator::bitwise_xor(expr_result auto lhs, expr_result auto rhs) noexcept
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

  void evaluator::bitwise_or(expr_result auto lhs, expr_result auto rhs) noexcept
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

  void evaluator::add(addable auto lhs, addable auto rhs) noexcept
  {
    visit_binary(std::move(lhs), std::move(rhs),
      [](auto l, auto r) noexcept
      {
        return l + r;
      });
  }
  void evaluator::add(expr_result auto, expr_result auto) noexcept
  {
    clear_result();
  }

  void evaluator::sub(subtractable auto lhs, subtractable auto rhs) noexcept
  {
    visit_binary(std::move(lhs), std::move(rhs),
      [](auto l, auto r) noexcept
      {
        return l - r;
      });
  }
  void evaluator::sub(expr_result auto, expr_result auto) noexcept
  {
    clear_result();
  }

  void evaluator::mul(multipliable auto lhs, multipliable auto rhs) noexcept
  {
    visit_binary(std::move(lhs), std::move(rhs),
      [](auto l, auto r) noexcept
      {
        return l * r;
      });
  }
  void evaluator::mul(expr_result auto, expr_result auto) noexcept
  {
    clear_result();
  }

  void evaluator::div(divisible auto lhs, divisible auto rhs) noexcept
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
  void evaluator::div(expr_result auto, expr_result auto) noexcept
  {
    clear_result();
  }

  // Modulo

  void evaluator::mod(fmod_divisible auto lhs, fmod_divisible auto rhs) noexcept
  {
    visit_binary(std::move(lhs), std::move(rhs),
      [](auto l, auto r) noexcept
      {
        return std::fmod(l, r);
      });
  }
  void evaluator::mod(modulo_divisible auto lhs, modulo_divisible auto rhs) noexcept
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
  void evaluator::mod(expr_result auto, expr_result auto) noexcept
  {
    clear_result();
  }

  // Pow and root

  template <expr_result T> requires (std::is_arithmetic_v<T>)
  auto evaluator::enforce_complex(const T& l, const T& r) noexcept -> typed_value<complex_type>
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
    const auto res = complex_type{ 0.0, std::pow(utils::abs(base), utils::inv(square)) };
    const auto intrm = utils::eq(utils::abs(remainder), 1.0) ? res : std::pow(res, remainder);
    return (remainder > 0.0) ? intrm : eval::inv(intrm);
  }
  auto evaluator::enforce_complex(const expr_result auto&, const expr_result auto&) noexcept
  {
    return typed_value<complex_type>{};
  }

  template <expr_result T> requires (std::is_arithmetic_v<T>)
  auto evaluator::neg_root(const T& l, const T& r) noexcept -> typed_value<float_type>
  {
    auto base = static_cast<float_type>(l);
    if (base > 0.0 || utils::eq(base, 0.0))
      return {};

    auto exp = static_cast<float_type>(r);
    if (const auto mod2 = std::fmod(utils::inv(exp), 2.0); utils::eq(mod2, 0.0))
      return {};

    return -std::pow(utils::abs(base), exp);
  }
  auto evaluator::neg_root(const expr_result auto&, const expr_result auto&) noexcept
  {
    return typed_value<float_type>{};
  }

  void evaluator::power(pow_raisable auto base, pow_raisable auto exp) noexcept
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
  void evaluator::power(expr_result auto base, expr_result auto exp) noexcept
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

  void evaluator::root(invertible auto base, invertible auto exp) noexcept
  {
    if constexpr (utils::same_noquals<decltype(base), int_type>)
      root(static_cast<float_type>(base), static_cast<float_type>(exp));
    else
      power(base, eval::inv(exp));
  }
  void evaluator::root(expr_result auto, expr_result auto) noexcept
  {
    clear_result();
  }

  // Relation and equality

  void evaluator::equal(eq_comparable auto lhs, eq_comparable auto rhs, bool compareForEquality) noexcept
  {
    const auto cmp = eval::eq(lhs, rhs);
    const auto res = compareForEquality ? cmp : !cmp;
    reg_value(res);
  }
  void evaluator::equal(array_type lhs, array_type rhs, bool compareForEquality) noexcept
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
  void evaluator::equal(expr_result auto, expr_result auto, bool) noexcept
  {
    clear_result();
  }

  void evaluator::less(rel_comparable auto lhs, rel_comparable auto rhs) noexcept
  {
    visit_binary(std::move(lhs), std::move(rhs),
      [](auto l, auto r) noexcept
      {
        return eval::less(l, r);
      });
  }
  void evaluator::less(array_type lhs, array_type rhs) noexcept
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
  void evaluator::less(expr_result auto, expr_result auto) noexcept
  {
    clear_result();
  }

  void evaluator::less_eq(fully_comparable auto lhs, fully_comparable auto rhs) noexcept
  {
    visit_binary(std::move(lhs), std::move(rhs),
      [](auto l, auto r) noexcept
      {
        return eval::eq(l, r) || eval::less(l, r);
      });
  }
  void evaluator::less_eq(array_type lhs, array_type rhs) noexcept
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
  void evaluator::less_eq(expr_result auto, expr_result auto) noexcept
  {
    clear_result();
  }

  void evaluator::greater(fully_comparable auto lhs, fully_comparable auto rhs) noexcept
  {
    visit_binary(std::move(lhs), std::move(rhs),
      [](auto l, auto r) noexcept
      {
        return !eval::eq(l, r) && !eval::less(l, r);
      });
  }
  void evaluator::greater(array_type lhs, array_type rhs) noexcept
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
  void evaluator::greater(expr_result auto, expr_result auto) noexcept
  {
    clear_result();
  }

  void evaluator::greater_eq(fully_comparable auto lhs, fully_comparable auto rhs) noexcept
  {
    visit_binary(std::move(lhs), std::move(rhs),
      [](auto l, auto r) noexcept
      {
        return !eval::less(l, r);
      });
  }
  void evaluator::greater_eq(array_type lhs, array_type rhs) noexcept
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
  void evaluator::greater_eq(expr_result auto, expr_result auto) noexcept
  { 
    clear_result();
  }
}

namespace tnac::eval // Operations main
{
  template <expr_result T, unary_function<T> F>
  void evaluator::visit_unary(T val, F&& op) noexcept
  {
    reg_value(op(std::move(val)));
  }

  template <expr_result T>
  void evaluator::visit_unary(T operand, val_ops op) noexcept
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
  void evaluator::visit_unary(array_type operand, val_ops op) noexcept
  {
    utils::unused(operand, op);
    //if (utils::eq_any(op, val_ops::LogicalIs, val_ops::LogicalNot))
    //{
    //  auto toBool = get_caster<bool_type>()(std::move(operand));
    //  return visit_unary(toBool.value_or(false), op);
    //}

    //value_lock _{ operand, m_registry };
    //for (auto&& el : *operand)
    //{
    //  visit_unary(*el, op);
    //}

    //make_array(operand->size());
  }


  template <expr_result L, expr_result R, binary_function<L, R> F>
  void evaluator::visit_binary(L lhs, R rhs, F&& op) noexcept
  {
    reg_value(op(std::move(lhs), std::move(rhs)));
  }

  template <expr_result L, expr_result R>
  void evaluator::visit_binary(L l, R r, val_ops op) noexcept
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
  void evaluator::visit_binary(array_type l, array_type r, val_ops op) noexcept
  {
    utils::unused(l, r, op);
    //if (detail::is_comparison(op))
    //{
    //  using enum val_ops;
    //  switch (op)
    //  {
    //  case RelLess:   less(std::move(l), std::move(r));         break;
    //  case RelLessEq: less_eq(std::move(l), std::move(r));      break;
    //  case RelGr:     greater(std::move(l), std::move(r));      break;
    //  case RelGrEq:   greater_eq(std::move(l), std::move(r));   break;
    //  case Equal:     equal(std::move(l), std::move(r), true);  break;
    //  case NEqual:    equal(std::move(l), std::move(r), false); break;
    //  }
    //  return;
    //}

    //static auto unitArr = []() noexcept { arr_t ret; ret.emplace_back(int_type{}); return ret; }();
    //const auto lsz = l->size();
    //if (!lsz) l = array_type{ unitArr };
    //const auto rsz = r->size();
    //if (!rsz) r = array_type{ unitArr };
    //const auto newSz = (lsz && rsz) ? lsz * rsz : std::max(lsz, rsz);

    //value_lock _l{ l, m_registry };
    //value_lock _r{ r, m_registry };

    //for (auto&& el : *l)
    //{
    //  for (auto&& er : *r)
    //  {
    //    visit_binary(*el, *er, op);
    //  }
    //}

    //make_array(newSz);
  }

  template <expr_result T> requires (!utils::same_noquals<T, array_type>)
  void evaluator::visit_binary(array_type l, T r, val_ops op) noexcept
  {
    auto rhs = to_unit_array(r);
    visit_binary(std::move(l), array_type{ rhs }, op);
  }

  template <expr_result T> requires (!utils::same_noquals<T, array_type>)
  void evaluator::visit_binary(T l, array_type r, val_ops op) noexcept
  {
    auto lhs = to_unit_array(l);
    visit_binary(array_type{ lhs }, std::move(r), op);
  }

  void evaluator::visit_binary(expr_result auto lhs, value rhs, val_ops op) noexcept
  {
    visit_value(rhs, [this, l = std::move(lhs), op](auto rhs) noexcept
      {
        visit_binary(std::move(l), std::move(rhs), op);
      });
  }
}

namespace tnac::eval
{
  // Special members

  evaluator::~evaluator() noexcept = default;

  evaluator::evaluator(registry& reg) noexcept :
    m_registry{ reg }
  {}


  // Public members

  void evaluator::instantiate(type_id type, size_type argSz) noexcept
  {
    switch (type)
    {
    case type_id::Bool:     instantiate<bool_type>(argSz); break;
    case type_id::Int:      instantiate<int_type>(argSz); break;
    case type_id::Float:    instantiate<float_type>(argSz); break;
    case type_id::Complex:  instantiate<complex_type>(argSz); break;
    case type_id::Fraction: instantiate<fraction_type>(argSz); break;

    default: break;
    }
  }

  value evaluator::make_function(entity_id ent, function_type f) noexcept
  {
    VALUE_GUARD(m_curEntity, ent);
    reg_value(f);
    return m_registry.value_for(m_curEntity);
  }

  void evaluator::visit_binary(value lhs, value rhs, val_ops op) noexcept
  {
    if (!detail::is_binary(op))
      clear_result();

    visit_value(lhs, [this, rhs, op](auto lhs) noexcept
      {
        visit_binary(std::move(lhs), std::move(rhs), op);
      });
  }

  void evaluator::visit_unary(value val, val_ops op) noexcept
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

  void evaluator::push_value(value val) noexcept
  {
    visit_value(val, [this](auto v) noexcept
      {
        reg_value(std::move(v));
      });
  }

  void evaluator::push_last() noexcept
  {
    push_value(last());
  }

  value evaluator::visit_assign(entity_id ent, value rhs) noexcept
  {
    VALUE_GUARD(m_curEntity, ent);

    visit_value(rhs, [this](auto v) noexcept
      {
        visit_assign(std::move(v));
      });

    return m_registry.value_for(m_curEntity);
  }

  void evaluator::visit_int_literal(string_t src, int base) noexcept
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

  void evaluator::visit_float_literal(string_t src) noexcept
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

  void evaluator::visit_bool_literal(bool value) noexcept
  {
    return reg_value(value);
  }

  void evaluator::visit_i() noexcept
  {
    return reg_value(complex_type{ 0, 1 });
  }

  void evaluator::visit_pi() noexcept
  {
    return reg_value(std::numbers::pi_v<float_type>);
  }

  void evaluator::visit_e() noexcept
  {
    return reg_value(std::numbers::e_v<float_type>);
  }

  stored_value evaluator::fetch_next() noexcept
  {
    return m_registry.consume();
  }

  value evaluator::last() noexcept
  {
    return m_registry.evaluation_result();
  }

  void evaluator::clear_result() noexcept
  {
    reg_value(eval::invalid_val_t{});
  }

}
#endif