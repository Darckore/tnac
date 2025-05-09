#include "eval/value/value.hpp"
#include "eval/value/traits.hpp"
#include "eval/value/type_impl.hpp"
#include "eval/value/value_store.hpp"

namespace tnac::eval::detail
{
  namespace
  {
    constexpr auto is_unary(val_ops op) noexcept
    {
      using enum val_ops;
      return utils::eq_any(op, UnaryPlus, UnaryNegation, UnaryBitwiseNot,
        LogicalNot, LogicalIs, AbsoluteValue, UnaryHead, PostTail);
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

namespace tnac::eval
{
  enum class cmp : std::uint8_t
  {
    Less,
    Equal,
    Greater
  };
}

namespace tnac::eval
{
  // Special members

  value::~value() noexcept = default;

  value::value() noexcept :
    value{ invalid_val_t{} }
  {
  }

  value& value::operator=(invalid_val_t inv) noexcept
  {
    m_raw = inv;
    return *this;
  }

  value::operator bool() const noexcept
  {
    return !std::holds_alternative<invalid_val_t>(m_raw);
  }


  // Public members

  type_id value::id() const noexcept
  {
    return static_cast<type_id>(m_raw.index());
  }

  string_t value::id_str(type_id id) noexcept
  {
    switch (id)
    {
    case Bool:     return "bool"sv;
    case Int:      return "int"sv;
    case Float:    return "float"sv;
    case Complex:  return "cplx"sv;
    case Fraction: return "frac"sv;
    case Function: return "fn"sv;
    case Array:    return "arr"sv;

    default: break;
    }

    return "undef"sv;
  }
  
  string_t value::id_str() const noexcept
  {
    return id_str(id());
  }

  value::size_type value::size() const noexcept
  {
    return size_of(id());
  }

  // Private members


  // Public members (evaluation)

  value value::parse_int(string_t src, int base) noexcept
  {
    auto prefix = string_t::size_type{};
    if (utils::eq_any(base, 2, 16))
      prefix = 2u;
    else if (base == 8)
      prefix = 1u;

    auto begin = src.data() + prefix;
    auto end = begin + src.length();

    value ret{};
    int_type result{};
    auto convRes = std::from_chars(begin, end, result, base);
    if (convRes.ec == std::errc{ 0 })
    {
      ret = result;
    }

    return ret;
  }

  value value::parse_float(string_t src) noexcept
  {
    auto begin = src.data();
    auto end = begin + src.length();

    value ret{};
    float_type result{};
    auto convRes = std::from_chars(begin, end, result);
    if (convRes.ec == std::errc{ 0 })
    {
      ret = result;
    }

    return ret;
  }

  value value::pi() noexcept
  {
    return value{ std::numbers::pi_v<float_type> };
  }

  value value::e() noexcept
  {
    return value{ std::numbers::e_v<float_type> };
  }

  value value::i() noexcept
  {
    return value{ complex_type{ 0, 1 } };
  }

  value value::true_val() noexcept
  {
    return value{ true };
  }

  value value::false_val() noexcept
  {
    return value{ false };
  }

  value value::function(ir::function& func) noexcept
  {
    return value{ function_type{ func } };
  }

  value value::array(array_wrapper& aw) noexcept
  {
    return value{ array_type{ aw } };
  }


  // Unary ops
  namespace
  {
    auto unary_plus(const plusable auto& operand) noexcept
    {
      return value{ +operand };
    }
    auto unary_plus(const expr_result auto&) noexcept
    {
      return value{};
    }

    auto unary_neg(const negatable auto& operand) noexcept
    {
      return value{ -operand };
    }
    auto unary_neg(const expr_result auto&) noexcept
    {
      return value{};
    }

    auto bitwise_not(const expr_result auto& operand) noexcept
    {
      auto intOp = get_caster<int_type>()(operand);
      return intOp ? value{ ~*intOp } : value{};
    }

    auto logical_not(const expr_result auto& operand) noexcept
    {
      auto boolOp = get_caster<bool_type>()(operand);
      return value{ !boolOp.value_or(false) };
    }

    auto logical_is(const expr_result auto& operand) noexcept
    {
      auto boolOp = get_caster<bool_type>()(operand);
      return value{ boolOp && *boolOp };
    }

    auto absolute(const abs_compatible auto& operand) noexcept
    {
      return value{ eval::abs(operand) };
    }
    auto absolute(const expr_result auto&) noexcept
    {
      return value{};
    }

    auto unary_head(const expr_result auto& operand) noexcept
    {
      return value{ eval::head(operand) };
    }
    auto unary_head(const array_type& arr) noexcept
    {
      return eval::head(arr);
    }

    auto unary_tail(const expr_result auto& operand) noexcept
    {
      return value{ eval::tail(operand) };
    }
    auto unary_tail(const array_type& arr) noexcept
    {
      return eval::tail(arr);
    }
  }

  value value::unary_as_array(val_ops op) const noexcept
  {
    auto arr = std::get<array_type>(m_raw);
    if (utils::eq_any(op, val_ops::LogicalIs, val_ops::LogicalNot))
    {
      auto toBool = get_caster<bool_type>()(std::move(arr));
      return value{ toBool.value_or(false) }.unary(op);
    }

    if (op == val_ops::UnaryHead)
      return unary_head(arr);

    if (op == val_ops::PostTail)
      return unary_tail(arr);

    auto&& store = arr->val_store();
    auto&& resData = store.allocate_array(arr->size());
    for (auto&& it : arr.wrapper())
    {
      resData.add(it.unary(op));
    }

    auto&& aw = store.wrap(resData);
    return value{ array_type{ aw } };
  }

  value value::unary(val_ops op) const noexcept
  {
    if (is_array())
      return unary_as_array(op);

    value res{};
    if (detail::is_unary(op))
    {
      res = std::visit([op](const auto& v) noexcept
        {
          using arg_t = std::remove_cvref_t<decltype(v)>;
          using op_type = common_type_t<arg_t, arg_t>;
          auto val = get_caster<op_type>()(v);
          if (!val)
            return value{};

          using enum val_ops;
          switch (op)
          {
          case UnaryNegation:   return unary_neg(*val);
          case UnaryPlus:       return unary_plus(*val);
          case UnaryBitwiseNot: return bitwise_not(*val);
          case LogicalNot:      return logical_not(*val);
          case LogicalIs:       return logical_is(*val);
          case AbsoluteValue:   return absolute(*val);
          case UnaryHead:       return unary_head(*val);
          case PostTail:        return unary_tail(*val);

          default: return value{};
          }
        },
        m_raw);
    }

    return res;
  }


  // Binary ops
  namespace
  {
    auto add(const addable auto& lhs, const addable auto& rhs) noexcept
    {
      return value{ lhs + rhs };
    }
    auto add(const expr_result auto&, const expr_result auto&) noexcept
    {
      return value{};
    }

    auto sub(const subtractable auto& lhs, const subtractable auto& rhs) noexcept
    {
      return value{ lhs - rhs };
    }
    auto sub(const expr_result auto&, const expr_result auto&) noexcept
    {
      return value{};
    }

    auto mul(const multipliable auto& lhs, const multipliable auto& rhs) noexcept
    {
      return value{ lhs * rhs };
    }
    auto mul(const expr_result auto&, const expr_result auto&) noexcept
    {
      return value{};
    }

    auto div(const divisible auto& lhs, const divisible auto& rhs) noexcept
    {
      if constexpr (utils::same_noquals<decltype(lhs), int_type>)
      {
        return div(static_cast<float_type>(lhs), static_cast<float_type>(rhs));
      }
      else
      {
        return value{ lhs / rhs };
      }
    }
    auto div(const expr_result auto&, const expr_result auto&) noexcept
    {
      return value{};
    }

    auto mod(const fmod_divisible auto& lhs, const fmod_divisible auto& rhs) noexcept
    {
      return value{ std::fmod(lhs, rhs) };
    }
    auto mod(const modulo_divisible auto& lhs, const modulo_divisible auto& rhs) noexcept
    {
      if constexpr (utils::same_noquals<decltype(lhs), int_type>)
      {
        return mod(static_cast<float_type>(lhs), static_cast<float_type>(rhs));
      }
      else
      {
        return value{ lhs % rhs };
      }
    }
    auto mod(const expr_result auto&, const expr_result auto&) noexcept
    {
      return value{};
    }

    cmp compare_arrays(array_type lhs, array_type rhs) noexcept
    {
      const auto lsz = lhs->size();
      const auto rsz = rhs->size();
      if (lhs->id() == rhs->id() && 
          lhs->offset() == rhs->offset() &&
          lsz == rsz)
      {
        return cmp::Equal;
      }

      auto res = cmp::Equal;
      for (auto&& [le, re] : utils::make_iterators(lhs.wrapper(), rhs.wrapper()))
      {
        if (to_bool(le.binary(val_ops::Equal, re)))
          continue;

        if (to_bool(le.binary(val_ops::RelLess, re)))
        {
          res = cmp::Less;
          break;
        }

        res = cmp::Greater;
        break;
      }

      if (res != cmp::Equal)
        return res;

      if (lsz < rsz)
        return cmp::Less;

      return cmp::Greater;
    }

    auto eq(const eq_comparable auto& lhs, const eq_comparable auto& rhs, bool compareForEquality) noexcept
    {
      const auto cmp = eval::eq(lhs, rhs);
      return value{ compareForEquality ? cmp : !cmp };
    }
    auto eq(const expr_result auto&, const expr_result auto&, bool) noexcept
    {
      return value{};
    }
    auto eq(array_type lhs, array_type rhs, bool compareForEquality) noexcept
    {
      const auto cmpRes = compare_arrays(std::move(lhs), std::move(rhs));
      if (cmpRes == cmp::Equal)
        return value{ compareForEquality };

      return value{ !compareForEquality };
    }

    auto lt(const rel_comparable auto& lhs, const rel_comparable auto& rhs) noexcept
    {
      return value{ eval::less(lhs, rhs) };
    }
    auto lt(const expr_result auto&, const expr_result auto&) noexcept
    {
      return value{};
    }
    auto lt(array_type lhs, array_type rhs) noexcept
    {
      const auto cmpRes = compare_arrays(std::move(lhs), std::move(rhs));
      return value{ cmpRes == cmp::Less };
    }

    auto lte(const fully_comparable auto& lhs, const fully_comparable auto& rhs) noexcept
    {
      return value{ eval::eq(lhs, rhs) || eval::less(lhs, rhs) };
    }
    auto lte(const expr_result auto&, const expr_result auto&) noexcept
    {
      return value{};
    }
    auto lte(array_type lhs, array_type rhs) noexcept
    {
      const auto cmpRes = compare_arrays(std::move(lhs), std::move(rhs));
      return value{ utils::eq_any(cmpRes, cmp::Less, cmp::Equal) };
    }

    auto gt(const fully_comparable auto& lhs, const fully_comparable auto& rhs) noexcept
    {
      return value{ !eval::eq(lhs, rhs) && !eval::less(lhs, rhs) };
    }
    auto gt(const expr_result auto&, const expr_result auto&) noexcept
    {
      return value{};
    }
    auto gt(array_type lhs, array_type rhs) noexcept
    {
      const auto cmpRes = compare_arrays(std::move(lhs), std::move(rhs));
      return value{ cmpRes == cmp::Greater };
    }

    auto gte(const rel_comparable auto& lhs, const rel_comparable auto& rhs) noexcept
    {
      return value{ !eval::less(lhs, rhs) };
    }
    auto gte(const expr_result auto&, const expr_result auto&) noexcept
    {
      return value{};
    }
    auto gte(array_type lhs, array_type rhs) noexcept
    {
      const auto cmpRes = compare_arrays(std::move(lhs), std::move(rhs));
      return value{ utils::eq_any(cmpRes, cmp::Greater, cmp::Equal) };
    }


    auto bit_and(const expr_result auto& lhs, const expr_result auto& rhs) noexcept
    {
      auto caster = get_caster<int_type>();
      auto intL = caster(lhs);
      auto intR = caster(rhs);
      value res{};
      if (intL && intR)
      {
        res = value{ *intL & *intR };
      }

      return res;
    }

    auto bit_xor(const expr_result auto& lhs, const expr_result auto& rhs) noexcept
    {
      auto caster = get_caster<int_type>();
      auto intL = caster(lhs);
      auto intR = caster(rhs);
      value res{};
      if (intL && intR)
      {
        res = value{ *intL ^ *intR };
      }

      return res;
    }

    auto bit_or(const expr_result auto& lhs, const expr_result auto& rhs) noexcept
    {
      auto caster = get_caster<int_type>();
      auto intL = caster(lhs);
      auto intR = caster(rhs);
      value res{};
      if (intL && intR)
      {
        res = value{ *intL | *intR };
      }

      return res;
    }


    template <expr_result T> requires (std::is_arithmetic_v<T>)
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
      const auto res = complex_type{ 0.0, std::pow(utils::abs(base), utils::inv(square)) };
      const auto intrm = utils::eq(utils::abs(remainder), 1.0) ? res : std::pow(res, remainder);
      return (remainder > 0.0) ? intrm : eval::inv(intrm);
    }
    auto enforce_complex(const expr_result auto&, const expr_result auto&) noexcept
    {
      return typed_value<complex_type>{};
    }

    template <expr_result T> requires (std::is_arithmetic_v<T>)
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
    auto neg_root(const expr_result auto&, const expr_result auto&) noexcept
    {
      return typed_value<float_type>{};
    }

    auto power(const pow_raisable auto& base, const pow_raisable auto& exp) noexcept
    {
      value res{};
      if (auto cpl = enforce_complex(base, exp))
      {
        res = *cpl;
      }
      else if (auto neg = neg_root(base, exp))
      {
        res = *neg;
      }
      else
      {
        res = std::pow(base, exp);
      }

      return res;
    }
    auto power(const expr_result auto& base, const expr_result auto& exp) noexcept
    {
      auto caster = get_caster<float_type>();
      auto floatL = caster(base);
      auto floatR = caster(exp);
      value res{};
      if (floatL && floatR)
      {
        res = power(*floatL, *floatR);
      }

      return res;
    }

    auto root(const invertible auto& base, const invertible auto& exp) noexcept
    {
      if constexpr (utils::same_noquals<decltype(base), int_type>)
        return root(static_cast<float_type>(base), static_cast<float_type>(exp));
      else
        return power(base, eval::inv(exp));
    }
    auto root(const expr_result auto&, const expr_result auto&) noexcept
    {
      return value{};
    }
  }

  value value::binary_as_array(val_ops op, const value& r) const noexcept
  {
    auto vs = extract_store();
    if (!vs)
      vs = r.extract_store();

    auto caster = get_caster<array_type>(vs);
    auto lhs = caster(*this);
    auto rhs = caster(r);
    if (!lhs || !rhs)
      return {};

    auto larr = std::move(*lhs);
    auto rarr = std::move(*rhs);
    if (detail::is_comparison(op))
    {
      using enum val_ops;
      switch (op)
      {
        case RelLess:   return lt(std::move(larr), std::move(rarr));
        case RelLessEq: return lte(std::move(larr), std::move(rarr));
        case RelGr:     return gt(std::move(larr), std::move(rarr));
        case RelGrEq:   return gte(std::move(larr), std::move(rarr));
        case Equal:     return eq(std::move(larr), std::move(rarr), true);
        case NEqual:    return eq(std::move(larr), std::move(rarr), false);
      }
    }

    const auto lsz = larr->size();
    const auto rsz = rarr->size();
    if (!lsz || !rsz)
      return {};

    UTILS_ASSERT(vs);
    auto&& resArr = vs->allocate_array(lsz * rsz);
    for (auto&& li : larr.wrapper())
    {
      for (auto&& ri : rarr.wrapper())
      {
        resArr.add(li.binary(op, ri));
      }
    }

    return value{ array_type{ vs->wrap(resArr) } };
  }

  value value::binary(val_ops op, const value& rhs) const noexcept
  {
    if (is_array() || rhs.is_array())
      return binary_as_array(op, rhs);

    value res{};
    if (detail::is_binary(op))
    {
      res = std::visit([op](const auto& l, const auto& r) noexcept
        {
          using lhs_t = std::remove_cvref_t<decltype(l)>;
          using rhs_t = std::remove_cvref_t<decltype(r)>;
          using common_t = common_type_t<lhs_t, rhs_t>;
          auto caster = get_caster<common_t>();
          auto lhs = caster(l);
          auto rhs = caster(r);
          if (!lhs || !rhs)
          {
            return value{};
          }

          using enum val_ops;
          switch (op)
          {
          case Addition:       return add(*lhs, *rhs);
          case Subtraction:    return sub(*lhs, *rhs);
          case Multiplication: return mul(*lhs, *rhs);
          case Division:       return div(*lhs, *rhs);
          case Modulo:         return mod(*lhs, *rhs);

          case RelLess:        return lt(*lhs, *rhs);
          case RelLessEq:      return lte(*lhs, *rhs);
          case RelGr:          return gt(*lhs, *rhs);
          case RelGrEq:        return gte(*lhs, *rhs);
          case Equal:          return eq(*lhs, *rhs, true);
          case NEqual:         return eq(*lhs, *rhs, false);

          case BitwiseAnd:     return bit_and(*lhs, *rhs);
          case BitwiseXor:     return bit_xor(*lhs, *rhs);
          case BitwiseOr:      return bit_or(*lhs, *rhs);

          case BinaryPow:      return power(*lhs, *rhs);
          case BinaryRoot:     return root(*lhs, *rhs);

          default: return value{};
          }
        },
        m_raw, rhs.m_raw);
    }

    return res;
  }


  // Private members

  bool value::is_array() const noexcept
  {
    return id() == type_id::Array;
  }

  store* value::extract_store() const noexcept
  {
    if (auto arr = try_get<array_type>())
      return &(*arr)->val_store();

    return {};
  }
}