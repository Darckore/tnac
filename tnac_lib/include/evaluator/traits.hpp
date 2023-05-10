//
// Type traits
//

#pragma once
#include "evaluator/value.hpp"

namespace tnac::eval
{
  // Type info

  namespace detail
  {
    template <std::size_t MinArgs, type_id... Args>
    struct basic_type_info
    {
      static constexpr std::array params{ Args... };
      static constexpr auto minArgs = MinArgs;
      static constexpr auto maxArgs = sizeof...(Args);
    };
  }

  template <typename T> struct type_info;

  template <>
  struct type_info<int_type> :
    detail::basic_type_info<1, type_id::Int>
  {};

  template <>
  struct type_info<float_type> :
    detail::basic_type_info<1, type_id::Float>
  {};

  template <>
  struct type_info<complex_type> :
    detail::basic_type_info<0, type_id::Float, type_id::Float>
  {};

  template <>
  struct type_info<fraction_type> :
    detail::basic_type_info<2, type_id::Int, type_id::Int>
  {};


  // Value casters

  template <typename T> struct cast_value;

  template <>
  struct cast_value<int_type>
  {
    int_type operator()(value val) noexcept
    {
      return on_value(val, utils::visitor
        {
          [](int_type v) noexcept { return v; },
          [](float_type v) noexcept { return static_cast<int_type>(v); },
          [](complex_type v) noexcept { return static_cast<int_type>(v.real()); },
          [](fraction_type v) noexcept
            {
              if (v.is_infinity())
              {
                using lim = std::numeric_limits<int_type>;
                return v.sign() > 0 ? lim::min() : lim::max();
              }
              return static_cast<int_type>(v.to<float_type>());
            },
          [](invalid_val_t) noexcept { return int_type{}; }
        });
    }
  };

  template <>
  struct cast_value<float_type>
  {
    float_type operator()(value val) noexcept
    {
      return on_value(val, utils::visitor
        {
          [](int_type v) noexcept { return static_cast<float_type>(v); },
          [](float_type v) noexcept { return v; },
          [](complex_type v) noexcept { return v.real(); },
          [](fraction_type v) noexcept { return v.to<float_type>(); },
          [](invalid_val_t) noexcept { return float_type{}; }
        });
    }
  };

  template <>
  struct cast_value<complex_type>
  {
    complex_type operator()(value val) noexcept
    {
      return on_value(val, utils::visitor
        {
          [](int_type v) noexcept { return complex_type{ static_cast<float_type>(v) }; },
          [](float_type v) noexcept { return complex_type{ v }; },
          [](complex_type v) noexcept { return v; },
          [](fraction_type v) noexcept { return complex_type{ v.to<float_type>() }; },
          [](invalid_val_t) noexcept { return complex_type{}; }
        });
    }
  };

  template <>
  struct cast_value<fraction_type>
  {
    fraction_type operator()(value val) noexcept
    {
      return on_value(val, utils::visitor
        {
          [](int_type v) noexcept { return fraction_type{ v }; },
          [](float_type v) noexcept { return fraction_type{ static_cast<int_type>(v) }; }, // todo: float to frac
          [](complex_type v) noexcept { return fraction_type{ static_cast<int_type>(v.real()) }; }, // todo: float to frac
          [](fraction_type v) noexcept { return v; },
          [](invalid_val_t) noexcept { return fraction_type{ 0 }; }
        });
    }
  };
}