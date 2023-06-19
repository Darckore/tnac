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
  struct type_info<bool_type> :
    detail::basic_type_info<0, type_id::Bool>
  {};

  template <>
  struct type_info<int_type> :
    detail::basic_type_info<0, type_id::Int>
  {};

  template <>
  struct type_info<float_type> :
    detail::basic_type_info<0, type_id::Float>
  {};

  template <>
  struct type_info<complex_type> :
    detail::basic_type_info<0, type_id::Float, type_id::Float>
  {};

  template <>
  struct type_info<fraction_type> :
    detail::basic_type_info<2, type_id::Int, type_id::Int>
  {};

  template <>
  struct type_info<function_type> :
    detail::basic_type_info<1, type_id::Function>
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
  struct cast_value<bool_type>
  {
    using res_type = typed_value<bool_type>;

    auto operator()(value val) noexcept
    {
      return on_value(val, utils::visitor
      {
        [](bool_type v) noexcept -> res_type
        {
          return v;
        },
        [](int_type v) noexcept -> res_type
        {
          return static_cast<bool>(v);
        },
        [](float_type v) noexcept -> res_type
        {
          return !utils::eq(v, float_type{});
        },
        [](complex_type v) noexcept -> res_type
        {
          constexpr auto cmp = float_type{};
          return !(utils::eq(v.real(), cmp) ||
                   utils::eq(v.imag(), cmp));
        },
        [](fraction_type v) noexcept -> res_type
        {
          return static_cast<bool>(v.num());
        },
        [](function_type) noexcept -> res_type
        {
          return true;
        },
        [](invalid_val_t) noexcept -> res_type
        {
          return false;
        }
      });
    }
  };


  template <>
  struct cast_value<int_type>
  {
    using res_type = typed_value<int_type>;

    auto operator()(value val) noexcept
    {
      return on_value(val, utils::visitor
        {
          [](bool_type v) noexcept -> res_type
          {
            return static_cast<int_type>(v);
          },
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
          [](function_type) noexcept -> res_type
          {
            return {};
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
          [](bool_type v) noexcept -> res_type
          {
            return v ? float_type{ 1.0 } : float_type{};
          },
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
          [](function_type) noexcept -> res_type
          {
            return {};
          },
          [](invalid_val_t) noexcept -> res_type
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
          [](bool_type v) noexcept -> res_type
          {
            return v ? complex_type{ 1.0, 0.0 } : complex_type{};
          },
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
          [](function_type) noexcept -> res_type
          {
            return {};
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
          [](bool_type v) noexcept -> res_type
          {
            return v ? fraction_type{ 1, 1 } : fraction_type{ 0, 1 };
          },
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
          [](function_type) noexcept -> res_type
          {
            return {};
          },
          [](invalid_val_t) noexcept -> res_type
          { 
            return fraction_type{ 0 };
          }
        });
    }
  };

  template <>
  struct cast_value<function_type>
  {
    using res_type = typed_value<function_type>;

    auto operator()(value val) noexcept
    {
      return on_value(val, utils::visitor
      {
        [](bool_type) noexcept -> res_type
        {
          return {};
        },
        [](int_type) noexcept -> res_type
        {
          return {};
        },
        [](float_type) noexcept -> res_type
        {
          return {};
        },
        [](complex_type) noexcept -> res_type
        {
          return {};
        },
        [](fraction_type) noexcept -> res_type
        {
          return {};
        },
        [](function_type v) noexcept -> res_type
        {
          return v;
        },
        [](invalid_val_t) noexcept -> res_type
        {
          return {};
        }
      });
    }
  };


  template <detail::expr_result T>
  inline typed_value<int_type> to_int(T) noexcept
  {
    return {};
  }

  template <>
  inline typed_value<int_type> to_int(bool_type val) noexcept
  {
    return static_cast<int_type>(val);
  }

  template <>
  inline typed_value<int_type> to_int(int_type val) noexcept
  {
    return val;
  }

  template <>
  inline typed_value<int_type> to_int(float_type val) noexcept
  {
    const auto conv = static_cast<int_type>(val);
    if (utils::eq(static_cast<float_type>(conv), val))
      return conv;

    return {};
  }

  template <>
  inline typed_value<int_type> to_int(complex_type val) noexcept
  {
    if (!utils::eq(val.imag(), float_type{}))
      return {};

    return to_int(val.real());
  }

  template <>
  inline typed_value<int_type> to_int(fraction_type val) noexcept
  {
    return to_int(val.to<float_type>());
  }


  template <detail::expr_result T>
  inline typed_value<float_type> to_float(T) noexcept
  {
    return {};
  }

  template <>
  inline typed_value<float_type> to_float(bool_type val) noexcept
  {
    return val ? float_type{ 1.0 } : float_type{};
  }

  template <>
  inline typed_value<float_type> to_float(int_type val) noexcept
  {
    return static_cast<float_type>(val);
  }

  template <>
  inline typed_value<float_type> to_float(float_type val) noexcept
  {
    return val;
  }

  template <>
  inline typed_value<float_type> to_float(complex_type val) noexcept
  {
    if (!utils::eq(val.imag(), float_type{}))
      return {};

    return to_float(val.real());
  }

  template <>
  inline typed_value<float_type> to_float(fraction_type val) noexcept
  {
    return val.to<float_type>();
  }


  //
  // Missing operators
  //
  inline complex_type operator+(const complex_type& l, int_type r) noexcept
  {
    return l + static_cast<float_type>(r);
  }
  inline complex_type operator+(int_type l, const complex_type& r) noexcept
  {
    return r + l;
  }
  inline complex_type operator+(const complex_type& l, bool_type r) noexcept
  {
    return l + static_cast<float_type>(r);
  }
  inline complex_type operator+(bool_type l, const complex_type& r) noexcept
  {
    return r + l;
  }
  inline complex_type operator+(const complex_type& l, const fraction_type& r) noexcept
  {
    return l + r.to<float_type>();
  }
  inline complex_type operator+(const fraction_type& l, const complex_type& r) noexcept
  {
    return r + l;
  }

  inline complex_type operator-(const complex_type& l, int_type r) noexcept
  {
    return l - static_cast<float_type>(r);
  }
  inline complex_type operator-(int_type l, const complex_type& r) noexcept
  {
    return static_cast<float_type>(l) - r;
  }
  inline complex_type operator-(const complex_type& l, bool_type r) noexcept
  {
    return l - static_cast<float_type>(r);
  }
  inline complex_type operator-(bool_type l, const complex_type& r) noexcept
  {
    return static_cast<float_type>(l) - r;
  }
  inline complex_type operator-(const complex_type& l, const fraction_type& r) noexcept
  {
    return l - r.to<float_type>();
  }
  inline complex_type operator-(const fraction_type& l, const complex_type& r) noexcept
  {
    return l.to<float_type>() - r;
  }

  inline complex_type operator*(const complex_type& l, int_type r) noexcept
  {
    return l * static_cast<float_type>(r);
  }
  inline complex_type operator*(int_type l, const complex_type& r) noexcept
  {
    return r * l;
  }
  inline complex_type operator*(const complex_type& l, bool_type r) noexcept
  {
    return l * static_cast<float_type>(r);
  }
  inline complex_type operator*(bool_type l, const complex_type& r) noexcept
  {
    return r * l;
  }
  inline complex_type operator*(const complex_type& l, const fraction_type& r) noexcept
  {
    return l * r.to<float_type>();
  }
  inline complex_type operator*(const fraction_type& l, const complex_type& r) noexcept
  {
    return r * l;
  }

  inline complex_type operator/(const complex_type& l, int_type r) noexcept
  {
    return l / static_cast<float_type>(r);
  }
  inline complex_type operator/(int_type l, const complex_type& r) noexcept
  {
    return static_cast<float_type>(l) / r;
  }
  inline complex_type operator/(const complex_type& l, bool_type r) noexcept
  {
    return l / static_cast<float_type>(r);
  }
  inline complex_type operator/(bool_type l, const complex_type& r) noexcept
  {
    return static_cast<float_type>(l) / r;
  }
  inline complex_type operator/(const complex_type& l, const fraction_type& r) noexcept
  {
    return l / r.to<float_type>();
  }
  inline complex_type operator/(const fraction_type& l, const complex_type& r) noexcept
  {
    return l.to<float_type>() / r;
  }

  template <detail::expr_result T> requires std::is_arithmetic_v<T>
  inline complex_type operator%(const complex_type& l, T r) noexcept
  {
    return { std::fmod(l.real(), static_cast<float_type>(r)),
             std::fmod(l.imag(), static_cast<float_type>(r)) };
  }
  template <detail::expr_result T> requires std::is_arithmetic_v<T>
  inline float_type operator%(const fraction_type& l, T r) noexcept
  {
    return std::fmod(l.to<float_type>(), static_cast<float_type>(r));
  }

  template <detail::expr_result T> requires std::is_arithmetic_v<T>
  inline float_type operator%(T l, const complex_type& r) noexcept
  {
    return std::fmod(static_cast<float_type>(l), r.real());
  }
  template <detail::expr_result T> requires std::is_arithmetic_v<T>
  inline float_type operator%(T l, const fraction_type& r) noexcept
  {
    return std::fmod(static_cast<float_type>(l), r.to<float_type>());
  }

  inline complex_type operator%(const complex_type& l, const complex_type& r) noexcept
  {
    const auto quotient = l / r;
    const auto mul = complex_type{ std::round(quotient.real()), std::round(quotient.imag()) };
    return l - mul * r;
  }
  inline float_type   operator%(const fraction_type& l, const fraction_type& r) noexcept
  {
    return l % r.to<float_type>();
  }

  inline complex_type operator%(const complex_type& l, const fraction_type& r) noexcept
  {
    return l % r.to<float_type>();
  }
  inline float_type   operator%(const fraction_type& l, const complex_type& r) noexcept
  {
    return l.to<float_type>() % r;
  }

}