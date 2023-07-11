#include "evaluator/value/value.hpp"

namespace tnac::eval
{
  // Statics

  namespace detail
  {
    struct val_maker
    {
      using val_t = value::value_type;
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

  value::value_type value::make(input_ptr ptr, type_id id) noexcept
  {
    auto res_val = reinterpret_cast<value_type>(ptr);
    const auto conv_id = static_cast<std::underlying_type_t<type_id>>(id);
    static_assert(sizeof(conv_id) == 1, "Type id is too large. Expected 1 byte");
    return detail::val_maker::add_id(res_val, conv_id);
  }

  value::id_val_pair value::split(value_type val) noexcept
  {
    return { 
      .val{ detail::val_maker::get_val(val) },
      .id{ detail::val_maker::get_id(val) }
    };
  }

  // Special members

  value::operator bool() const noexcept
  {
    return id() != type_id::Invalid;
  }


  // Public members

  type_id value::id() const noexcept
  {
    return split(m_val).id;
  }

  value::value_type value::raw() const noexcept
  {
    return split(m_val).val;
  }


  // Private members


}