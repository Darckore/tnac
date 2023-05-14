//
// Definitions common to all modules
//

#pragma once

/*
* Grammar
* 
*   scope:
*     expr-list
* 
*   expr-list:
*     expr
*     expr-list : expr
* 
*   expr:
*     decl-expr
* 
*   decl-expr:
*     var-decl
*     func-decl
*     assign-expr
* 
*   assign-expr:
*     additive_expr
*     id-expr assign-operator assign-expr
* 
*   bit-or-expr:
*     bit-xor-expr
*     bit-or-expr '|' bit-xor-expr
* 
*   bit-xor-expr:
*     bit-and-expr
*     bit-xor-expr '^' bit-and-expr
* 
*   bit-and-expr:
*     additive-expr
*     bit-and-expr '&' additive-expr
* 
*   additive-expr:
*     multiplicative-expr
*     additive-expr additive-operator multiplicative-expr
* 
*   multiplicative-expr:
*     unary-expr
*     multiplicative-expr multiplicative-operator unary-expr
* 
*   unary-expr:
*     primary-expr
*     unary-operator primary-expr
* 
*   primary-expr:
*     result-expr
*     literal-expr
*     id-expr
*     typed-expr
*     call-expr
*     '(' expr ')'
* 
*   var-decl:
*     undeclared-identifier '=' expr
* 
*   func-decl:
*     undeclared-identifier func-params func-body
* 
*   func-body:
*     expr-list ';'
* 
*   func-params:
*     '(' func-param-list ')'
* 
*   func-param-list:
*     param-decl
*     func-param-list ',' param-decl
* 
*   param-decl:
*     undeclared-identifier
* 
*   undeclared-identifier:
*     identifier (not previously declared)
* 
*   result-expr:
*     _result
* 
*   id-expr:
*     identifier (previously declared)
* 
*   literal-expr:
*     number
* 
*   call-expr:
*    identifier args
* 
*   typed-expr:
*     type-name args
* 
*   type-name:
*     type-keyword
* 
*   type-keyword: one of
*     _complex
*     _fraction
* 
*   args:
*     '(' arg-list ')'
* 
*   arg-list:
*     ''
*     expr
*     arg-list ',' expr
* 
*   command:
*     # id-sequence
* 
*   keyword:
*     _ id-sequence
* 
*   identifier:
*     identifier-start id-sequence
* 
*   number:
*     int-num
*     float-num
* 
*   float-num:
*     digit-sequence [.] digit-sequence
* 
*   int-num:
*     binary-int-num
*     octal-int-num
*     decimal-int-num
*     hex-int-num
* 
*   binary-int-num:
*     '0b' binary-digit-sequence
*
*   oct-int-num:
*     oct-digit-sequence
* 
*   decimal-int-num:
*     non-zero-digit digit-sequence
* 
*   hex-int-num:
*     '0x' hex-digit-sequence
* 
*   binary-digit-sequence:
*     binary-digit
*     binary-digit-sequence binary-digit
* 
*   oct-digit-sequence:
*     oct-digit
*     oct-digit-sequence oct-digit
* 
*   digit-sequence:
*     digit
*     digit-sequence digit
* 
*   hex-digit-sequence:
*     hex-digit
*     hex-digit-sequence hex-digit
* 
*   assign-operator: one of
*     =
* 
*   multiplicative-operator: one of
*     * / %
* 
*   additive-operator: one of
*     + -
* 
*   unary-operator: one of
*     + - ~
* 
*   operator: one of
*     + - ~ * / % | ^ & =
* 
*   string-literal:
*     ' anything '
* 
*   id-sequence:
*     identifier-char
*     id-sequence identifier-char
* 
*   identifier-start: one of
*     a b c d e f g h i j k l m n o p q r s t u v w x y z
*     A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
* 
*   identifier-char: one of
*     a b c d e f g h i j k l m n o p q r s t u v w x y z
*     A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
*     _ 1 2 3 4 5 6 7 8 9 0
* 
*   binary-digit: one of
*     0 1
* 
*   oct-digit: one of
*     0 1 2 3 4 5 6 7
* 
*   non-zero-digit: one of
*     1 2 3 4 5 6 7 8 9
* 
*   digit: one of
*     0 1 2 3 4 5 6 7 8 9
*
*   hex-digit: one of
*     0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F
* 
*/

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

    template <typename T1, typename T2>
    struct common_type : public std::common_type<nocvref<T1>, nocvref<T2>>
    {};

    template <typename From, typename To>
    inline constexpr auto to(From val) noexcept
    {
      return static_cast<To>(val);
    }
  }

  template <typename T1, typename T2>
  inline constexpr auto is_same_noquals_v = detail::is_same_noquals<T1, T2>::value;

  template <typename First, typename ...Others>
  inline constexpr auto is_any_v = detail::is_any<First, Others...>::value;

  template <typename First, typename ...Others>
  inline constexpr auto is_all_v = detail::is_all<First, Others...>::value;

  template <typename T1, typename T2>
  using common_type_t = detail::common_type<T1, T2>::type;

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