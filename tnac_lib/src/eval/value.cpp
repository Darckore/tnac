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

}