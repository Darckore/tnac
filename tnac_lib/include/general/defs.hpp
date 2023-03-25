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
*     multiplicative-expr
* 
*   multiplicative-expr:
*     unary-expr
*     unary-expr multiplicative-operator expr
* 
*   unary-expr:
*     primary-expr
*     unary-operator primary-expr
* 
*   primary-expr:
*     literal-expr
* 
*   literal-expr:
*     number
* 
*   number:
*     int-num
*     float-num
* 
*   float-num
*     digit-sequence [.] digit-sequence
* 
*   int-num:
*     binary-int-num
*     octal-int-num
*     decimal-int-num
*     hex-int-num
* 
*   binary-int-num
*     '0b' binary-digit-sequence
*
*   oct-int-num
*     oct-digit-sequence
* 
*   decimal-int-num:
*     non-zero-digit digit-sequence
* 
*   hex-int-num
*     '0x' hex-digit-sequence
* 
*   binary-digit-sequence
*     binary-digit
*     binary-digit-sequence binary-digit
* 
*   oct-digit-sequence
*     oct-digit
*     oct-digit-sequence oct-digit
* 
*   digit-sequence
*     digit
*     digit-sequence digit
* 
*   hex-digit-sequence
*     hex-digit
*     hex-digit-sequence hex-digit
* 
*   multiplicative-operator: one of
*     * /
* 
*   additive-operator: one of
*     + -
* 
*   unary-operator: one of
*     + -
* 
*   operator: one of
*     + - * /
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

namespace tnac
{
  using buf_t    = std::string;
  using string_t = std::string_view;
  using char_t   = string_t::value_type;

  using int_type   = std::intmax_t;
  using float_type = double;
}