//
// Definitions common to all modules
//

#pragma once

#define FROM_CONST(func, ...) utils::mutate(std::as_const(*this).func(__VA_ARGS__))

namespace tnac
{
  using buf_t    = std::string;
  using string_t = std::string_view;
  using char_t   = string_t::value_type;

  using int_type      = std::intmax_t;
  using float_type    = double;
  using complex_type  = std::complex<float_type>;
  using fraction_type = utils::ratio<int_type>;

  namespace detail
  {
    template <typename T>
    using nocvref = std::remove_cvref_t<T>;

    template <typename T1, typename T2>
    struct is_same_noquals
    {
      using t1 = nocvref<T1>;
      using t2 = nocvref<T2>;
      static constexpr auto value = std::is_same_v<t1, t2>;
    };

    template <typename First, typename ...Others>
    struct is_any
    {
      static constexpr auto value = std::disjunction_v<is_same_noquals<First, Others>...>;
    };

    template <typename First, typename ...Others>
    struct is_all
    {
      static constexpr auto value = std::conjunction_v<is_same_noquals<First, Others>...>;
    };
  }

  template <typename T1, typename T2>
  inline constexpr auto is_same_noquals_v = detail::is_same_noquals<T1, T2>::value;

  template <typename First, typename ...Others>
  inline constexpr auto is_any_v = detail::is_any<First, Others...>::value;

  template <typename First, typename ...Others>
  inline constexpr auto is_all_v = detail::is_all<First, Others...>::value;

  //
  // Takes a reference to a variable and (possibly) a new value
  // Resets the old value on scope exit
  //
  template <typename T>
  class value_guard
  {
  public:
    using value_type = T;
    using reference = value_type&;

  public:
    CLASS_SPECIALS_NONE(value_guard);

    value_guard(reference var) noexcept :
      m_ref{ var },
      m_old{ var }
    {}

    value_guard(reference var, value_type newVal) noexcept :
      m_ref{ var },
      m_old{ std::move(var) }
    {
      var = std::move(newVal);
    }

    ~value_guard() noexcept
    {
      m_ref = std::move(m_old);
    }

  private:
    reference m_ref;
    value_type m_old;
  };
}