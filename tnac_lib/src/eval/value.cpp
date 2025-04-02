#include "eval/value.hpp"
#include "eval/traits.hpp"

namespace tnac::eval
{
  // Statics

  namespace detail
  {
    struct val_maker
    {
      using val_t = old_crap_value::value_type;
      using id_t  = std::uint8_t;

      static constexpr auto bitsInByte = std::size_t{ 8 };
      static constexpr auto byteSz = sizeof(id_t);
      static constexpr auto valueSz = sizeof(val_t);
      static constexpr auto offset = (valueSz - byteSz) * bitsInByte;


      static constexpr auto get_shifted(id_t id) noexcept
      {
        const auto resVal = static_cast<val_t>(id) << offset;
        return resVal;
      }

      static constexpr auto mask() noexcept
      {
        return ~get_shifted(static_cast<val_t>(0xFF));
      }

      static constexpr auto get_id(val_t val) noexcept
      {
        return static_cast<type_id>(val >> offset);
      }

      static constexpr auto get_val(val_t val) noexcept
      {
        return val & mask();
      }

      static constexpr auto add_id(val_t val, id_t id) noexcept
      {
        const auto shiftedId = get_shifted(id);
        return get_val(val) | shiftedId;
      }
    };
  }

  old_crap_value::value_type old_crap_value::make(input_ptr ptr, type_id id) noexcept
  {
    auto res_val = reinterpret_cast<value_type>(ptr);
    const auto conv_id = static_cast<std::underlying_type_t<type_id>>(id);
    static_assert(sizeof(conv_id) == 1, "Type id is too large. Expected 1 byte");
    return detail::val_maker::add_id(res_val, conv_id);
  }

  old_crap_value::id_val_pair old_crap_value::split(value_type val) noexcept
  {
    return { 
      .val{ detail::val_maker::get_val(val) },
      .id{ detail::val_maker::get_id(val) }
    };
  }

  old_crap_value old_crap_value::zero() noexcept
  {
    static const auto i0 = int_type{};
    return old_crap_value{ &i0 };
  }


  // Special members

  old_crap_value::operator bool() const noexcept
  {
    return id() != type_id::Invalid;
  }


  // Public members

  type_id old_crap_value::id() const noexcept
  {
    return split(m_val).id;
  }

  old_crap_value::value_type old_crap_value::raw() const noexcept
  {
    return split(m_val).val;
  }

  old_crap_value::size_type old_crap_value::size() const noexcept
  {
    return size_of(id());
  }

  string_t old_crap_value::id_str() const noexcept
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

  // Private members


}