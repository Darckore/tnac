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
*     additive-expr
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
*     literal-expr
*     (expr)
* 
*   literal-expr:
*     number
* 
*   command:
*     # identifier
* 
*   keyword:
*     _ identifier
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

namespace tnac
{
  using buf_t    = std::string;
  using string_t = std::string_view;
  using char_t   = string_t::value_type;

  using int_type   = std::intmax_t;
  using float_type = double;
}