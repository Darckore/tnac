//
// Definitions common to all modules
//

#pragma once

namespace tnac
{
  using buf_t    = std::string;
  using string_t = std::string_view;
  using char_t   = string_t::value_type;

  namespace detail
  {
    template <typename T>
    using nocvref = std::remove_cvref_t<T>;

    template <typename T1, typename T2>
    struct is_same_noquals
    {
      using t1 = nocvref<T1>;
      using t2 = nocvref<T2>;
      static constexpr auto value = std::same_as<t1, t2>;
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
}