//
// Definitions common to all modules
//

#pragma once

/*
* Grammar
* 
*   number:
*     int-num
*     float-num
* 
*   float-num
*     digit-sequence [.] digit-sequence
* 
*   int-num:
*     decimal-int-num
* 
*   decimal-int-num:
*     non-zero-digit digit-sequence
* 
*   digit-sequence
*     digit
*     digit-sequence digit
* 
*   operator: one of
*     + - * /
* 
*   non-zero-digit: one of
*     1 2 3 4 5 6 7 8 9
* 
*   digit: one of
*     0 1 2 3 4 5 6 7 8 9
*/

namespace tnac
{
  using buf_t    = std::string;
  using string_t = std::string_view;
  using char_t   = string_t::value_type;

  using int_type   = std::intmax_t;
  using float_type = double;
}