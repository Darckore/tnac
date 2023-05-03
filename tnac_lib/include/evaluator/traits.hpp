//
// Type traits
//

#pragma once
#include "evaluator/value.hpp"

namespace tnac::eval
{
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
}