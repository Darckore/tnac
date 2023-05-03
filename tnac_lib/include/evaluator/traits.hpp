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
    template <std::size_t MinArgs, type_id... Params>
    struct basic_type_info
    {
      static constexpr std::array params{ Params... };
      static constexpr auto minArgs = MinArgs;
      static constexpr auto maxArgs = sizeof...(Params);
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
          [](invalid_val_t) noexcept { return complex_type{}; }
        });
    }
  };
}