#include "eval/value.hpp"
#include "eval/traits.hpp"

namespace tnac::eval
{
  // Statics


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

  string_t value::id_str() const noexcept
  {
    switch (id())
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

}