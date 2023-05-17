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

  template <detail::expr_result T>
  using typed_value = std::optional<T>;

  template <detail::expr_result T>
  struct type_wrapper
  {
    using value_type  = T;
    using result_type = typed_value<value_type>;

    template <typename ...Args>
    result_type operator()(typed_value<Args>&& ...args ) noexcept
    {
      if (utils::eq_any(false, static_cast<bool>(args)...))
      {
        return {};
      }

      return value_type{ (*args)... };
    }
  };

  template <typename T> struct cast_value;

  template <>
  struct cast_value<int_type>
  {
    using res_type = typed_value<int_type>;

    auto operator()(value val) noexcept
    {
      return on_value(val, utils::visitor
        {
          [](int_type v) noexcept -> res_type
          {
            return v;
          },
          [](float_type v) noexcept -> res_type
          {
            if (std::isnan(v) || std::isinf(v))
              return {};

            return static_cast<int_type>(v);
          },
          [](complex_type v) noexcept -> res_type
          {
            if (!utils::eq(v.imag(), float_type{}))
              return {};

            const auto real = v.real();
            if (std::isnan(real) || std::isinf(real))
              return {};

            return static_cast<int_type>(v.real());
          },
          [](fraction_type v) noexcept -> res_type
            {
              if (v.is_infinity())
              {
                return {};
              }
              return static_cast<int_type>(v.to<float_type>());
            },
          [](invalid_val_t) noexcept -> res_type
          { 
            return int_type{};
          }
        });
    }
  };

  template <>
  struct cast_value<float_type>
  {
    using res_type = typed_value<float_type>;

    auto operator()(value val) noexcept
    {
      return on_value(val, utils::visitor
        {
          [](int_type v) noexcept -> res_type
          { 
            return static_cast<float_type>(v);
          },
          [](float_type v) noexcept -> res_type
          { 
            return v; 
          },
          [](complex_type v) noexcept -> res_type
          { 
            if (!utils::eq(v.imag(), float_type{}))
              return {};

            return v.real(); 
          },
          [](fraction_type v) noexcept -> res_type
          { 
            return v.to<float_type>();
          },
          [](invalid_val_t) noexcept  -> res_type
          { 
            return float_type{};
          }
        });
    }
  };

  template <>
  struct cast_value<complex_type>
  {
    using res_type = typed_value<complex_type>;

    auto operator()(value val) noexcept
    {
      return on_value(val, utils::visitor
        {
          [](int_type v) noexcept -> res_type
          { 
            return complex_type{ static_cast<float_type>(v) };
          },
          [](float_type v) noexcept -> res_type
          { 
            return complex_type{ v };
          },
          [](complex_type v) noexcept -> res_type
          { 
            return v; 
          },
          [](fraction_type v) noexcept -> res_type
          { 
            return complex_type{ v.to<float_type>() };
          },
          [](invalid_val_t) noexcept -> res_type
          { 
            return complex_type{};
          }
        });
    }
  };

  template <>
  struct cast_value<fraction_type>
  {
    using res_type = typed_value<fraction_type>;

    auto operator()(value val) noexcept
    {
      return on_value(val, utils::visitor
        {
          [](int_type v) noexcept -> res_type
          {
            return fraction_type{ v };
          },
          [](float_type v) noexcept -> res_type
          {
            // todo: float to frac
            if (std::isnan(v) || std::isinf(v))
            {
              const auto sign = static_cast<fraction_type::sign_t>(utils::sign(v));
              return fraction_type{ int_type{ 1 }, int_type{ }, sign };
            }

            return fraction_type{ static_cast<int_type>(v) };
          },
          [](complex_type v) noexcept -> res_type
          {
            // todo: float to frac
            if (!utils::eq(v.imag(), float_type{}))
              return {};

            if (const auto real = v.real(); std::isnan(real) || std::isinf(real))
            {
              const auto sign = static_cast<fraction_type::sign_t>(utils::sign(real));
              return fraction_type{ int_type{ 1 }, int_type{ }, sign };
            }

            return fraction_type{ static_cast<int_type>(v.real()) };
          }, 
          [](fraction_type v) noexcept -> res_type
          {
            return v;
          },
          [](invalid_val_t) noexcept -> res_type
          { 
            return fraction_type{ 0 };
          }
        });
    }
  };
}