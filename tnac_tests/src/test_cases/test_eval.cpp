#include "test_cases/test_common.hpp"

namespace tnac_tests
{
  namespace
  {
    using vc = value_checker;
  }

  TEST(evaluation, t_constants)
  {
    vc::check("_true"sv, true);
    vc::check("_false"sv, false);
    vc::check("_i"sv, cplx{ 0, 1 });
    vc::check("_pi"sv, std::numbers::pi);
    vc::check("_e"sv, std::numbers::e);
  }

  TEST(evaluation, t_basic_add)
  {
    // Int
    vc::check("_true + 1"sv, 2ll);
    vc::check("1 + 2"sv, 3ll);
    vc::check("1 + 2.0"sv, 3.0);
    vc::check("1 + _cplx(1, 2)"sv, cplx{ 2.0, 2.0 });
    vc::check("_cplx(1, 2) + 2"sv, cplx{ 3.0, 2.0 });
    vc::check("2 + _frac(1, 2)"sv, frac{ 5, 2 });
    vc::check("_frac(1, 2) + 2"sv, frac{ 5, 2 });

    // Float
    vc::check("_true + 41.0"sv, 42.0);
    vc::check("4.0 + 5.0"sv, 9.0);
    vc::check("4.0 + 5"sv, 9.0);
    vc::check("4.0 + _cplx(1.0, 6)"sv, cplx{ 5.0, 6.0 });
    vc::check("_cplx(2.0, 6) + 5.0"sv, cplx{ 7.0, 6.0 });
    vc::check("4.0 + _frac(1, 2)"sv, 4.5);
    vc::check("_frac(3, 6) + 5.0"sv, 5.5);

    // Complex
    vc::check("_true + _cplx(4, 4)"sv, cplx{ 5.0, 4.0 });
    vc::check("_cplx(7, 10) + _cplx(10, 11)"sv, cplx{ 17.0, 21.0 });
    vc::check("_cplx(7, 10) + _frac(5, 10)"sv, cplx{ 7.5, 10.0 });

    // Fraction
    vc::check("_frac(1,2) + _frac(1,3)"sv, frac{ 5, 6 });
  }

  TEST(evaluation, t_basic_sub)
  {
    // Int
    vc::check("_false - 1"sv, -1ll);
    vc::check("1 - 2"sv, -1ll);
    vc::check("1 - 2.0"sv, -1.0);
    vc::check("1 - _cplx(1, 2)"sv, cplx{ 0.0, -2.0 });
    vc::check("_cplx(1, 2) - 2"sv, cplx{ -1.0, 2.0 });
    vc::check("2 - _frac(1, 2)"sv, frac{ 3, 2 });
    vc::check("_frac(1, 2) - 2"sv, frac{ 3, 2, -1 });

    // Float
    vc::check("42.0 - _true"sv, 41.0);
    vc::check("4.0 - 5.0"sv, -1.0);
    vc::check("6.0 - 5"sv, 1.0);
    vc::check("4.0 - _cplx(1.0, 6)"sv, cplx{ 3.0, -6.0 });
    vc::check("_cplx(2.0, 6) - 5.0"sv, cplx{ -3.0, 6.0 });
    vc::check("4.0 - _frac(1, 2)"sv, 3.5);
    vc::check("_frac(3, 6) - 5.0"sv, -4.5);

    // Complex
    vc::check("_cplx(7, 10) - _true"sv, cplx{ 6.0, 10.0 });
    vc::check("_cplx(7, 11) - _cplx(10, 11)"sv, cplx{ -3.0, 0.0 });
    vc::check("_cplx(7, 10) - _frac(5, 10)"sv, cplx{ 6.5, 10.0 });

    // Fraction
    vc::check("_frac(1,2) - _frac(1,3)"sv, frac{ 1, 6 });
  }

  TEST(evaluation, t_basic_mul)
  {
    // Int
    vc::check("1 * 2"sv, 2ll);
    vc::check("1 * 2.0"sv, 2.0);
    vc::check("1 * _cplx(1, 2)"sv, cplx{ 1.0, 2.0 });
    vc::check("_cplx(1, 2) * 2"sv, cplx{ 2.0, 4.0 });
    vc::check("2 * _frac(1, 2)"sv, frac{ 1, 1 });
    vc::check("_frac(1, 2) * 2"sv, frac{ 1, 1 });

    // Float
    vc::check("4.0 * 5.0"sv, 20.0);
    vc::check("6.0 * 5"sv, 30.0);
    vc::check("4.0 * _cplx(1.0, 6)"sv, cplx{ 4.0, 24.0 });
    vc::check("_cplx(2.0, 6) * 5.0"sv, cplx{ 10.0, 30.0 });
    vc::check("4.0 * _frac(1, 2)"sv, 2.0);
    vc::check("_frac(3, 6) * 5.0"sv, 2.5);

    // Complex
    vc::check("_cplx(7, 11) * _cplx(10, 11)"sv, cplx{ -51.0, 187.0 });
    vc::check("_cplx(7, 10) * _frac(5, 10)"sv, cplx{ 3.5, 5.0 });

    // Fraction
    vc::check("_frac(1,2) * _frac(1,3)"sv, frac{ 1, 6 });
  }

  TEST(evaluation, t_basic_div)
  {
    // Int
    vc::check("2 / 2"sv, 1.0);
    vc::check("1 / 0"sv, vc::infinity());
    vc::check("1 / _cplx(1, 2)"sv, cplx{ 0.2, -0.4 });
    vc::check("_cplx(1, 2) / 2"sv, cplx{ 0.5, 1.0 });
    vc::check("2 / _frac(1, 2)"sv, frac{ 4, 1 });
    vc::check("_frac(1, 2) / 2"sv, frac{ 1, 4 });

    // Float
    vc::check("4.0 / 5.0"sv, 0.8);
    vc::check("6.0 / 5"sv, 1.2);
    vc::check("4.0 / _cplx(2.0, 4)"sv, cplx{ 0.4, -0.8 });
    vc::check("_cplx(2.0, 6) / 5.0"sv, cplx{ 0.4, 1.2 });
    vc::check("4.0 / _frac(1, 2)"sv, 8.0);
    vc::check("_frac(3, 6) / 5.0"sv, 0.1);

    // Complex
    vc::check("_cplx(-51, 187) / _cplx(10, 11)"sv, cplx{ 7.0, 11.0 });
    vc::check("_cplx(7, 10) / _frac(5, 10)"sv, cplx{ 14.0, 20.0 });

    // Fraction
    vc::check("_frac(1,2) / _frac(1,3)"sv, frac{ 3, 2 });
  }

  TEST(evaluation, t_basic_mod)
  {
    // Int
    vc::check("2 % 2"sv, 0.0);
    vc::check("1 % 0"sv, vc::nan());
    vc::check("1 % _cplx(1, 2)"sv, cplx{ 1.0, 0.0 });
    vc::check("_cplx(1, 2) % 2"sv, cplx{ -1.0, 0.0 });
    vc::check("_frac(1, 1) % _cplx(1, 2)"sv, cplx{ 1.0, 0.0 });
    vc::check("_cplx(1, 2) % _frac(2, 1)"sv, cplx{ -1.0, 0.0 });

    // Float
    vc::check("4.0 % 5.0"sv, 4.0);
    vc::check("6.0 % 5"sv, 1.0);
    vc::check("4.0 % _cplx(2.0, 4)"sv, cplx{ 0.0, 2.0 });
    vc::check("_cplx(2.0, 6) % 5.0"sv, cplx{ 2.0, 1.0 });
    vc::check("_frac(4, 1) % _cplx(2.0, 4)"sv, cplx{ 0.0, 2.0 });
    vc::check("_cplx(2.0, 6) % _frac(5, 1)"sv, cplx{ 2.0, 1.0 });

    // Complex
    vc::check("_cplx(26, 120) % _cplx(37, 226)"sv, cplx{ -11.0, -106.0 });

    // Fraction
    vc::check("_frac(5, 1) % _frac(3, 1)"sv, 2.0);
  }

  TEST(evaluation, t_basic_pow)
  {
    vc::check("2 ** 2"sv, 4.0);
    vc::check("1 ** 0"sv, 1.0);
    vc::check("1 ** _cplx(1, 2)"sv, cplx{ 1.0, 0.0 });
    vc::check("_frac(1, 1) ** _cplx(1, 2)"sv, cplx{ 1.0, 0.0 });
    vc::check("4.0 ** 5.0"sv, 1024.0);
    vc::check("6.0 ** 5"sv, 7776.0);
    vc::check("_frac(8, 2) ** _frac(1, 2)"sv, 2.0);

    vc::check("-1 ** 0.5"sv, cplx{ 0.0, 1.0 });
    vc::check("-4 // 2"sv,   cplx{ 0.0, 2.0 });
    vc::check("-4 // -2"sv,  cplx{ 0.0, 0.5 });
  }

  TEST(evaluation, t_literal)
  {
    vc::check("2"sv, 2ll);
    vc::check("0b101"sv, 5ll);
    vc::check("010"sv, 8ll);
    vc::check("0xff"sv, 255ll);
    vc::check("42.69"sv, 42.69);
  }

  TEST(evaluation, t_unary)
  {
    vc::check("-2"sv, -2ll);
    vc::check("+42.69"sv, 42.69);
    vc::check("-(2 + 3)"sv, -5ll);
    vc::check("-(2/4)"sv, -0.5);
    vc::check("a = 10 : -a"sv, -10ll);
  }

  TEST(evaluation, t_absolute)
  {
    vc::check("| _true |"sv, 1ll);
    vc::check("| _false |"sv, 0ll);
    vc::check("| 2 |"sv, 2ll);
    vc::check("| -2 |"sv, 2ll);
    vc::check("| 2.0 |"sv, 2.0);
    vc::check("| -2.0 |"sv, 2.0);
    vc::check("| -2.0 |"sv, 2.0);
    vc::check("| _frac(1, 2) |"sv, detail::frac{1, 2});
    vc::check("| _frac(-1, 2) |"sv, detail::frac{1, 2});
    vc::check("| _cplx(3, 4) |"sv, 5.0);
    
    vc::check("| _fn(); + 1 |"sv);
  }

  TEST(evaluation, t_log_not)
  {
    vc::check("!_false"sv, true);
    vc::check("!_true"sv,  false);
    vc::check("!0"sv,  true);
    vc::check("!42"sv, false);
    vc::check("!0.0"sv, true);
    vc::check("!(-42.69)"sv, false);
    vc::check("!_frac(0, 42)"sv, true);
    vc::check("!_frac(1, 42)"sv, false);
    vc::check("!_cplx(0, 0)"sv, true);
    vc::check("!_cplx(0, 1)"sv, false);
    vc::check("!_cplx(1, 0)"sv, false);
    vc::check("f() ; !f"sv, false);
  }

  TEST(evaluation, t_log_is)
  {
    vc::check("?_false"sv, false);
    vc::check("?_true"sv, true);
    vc::check("?0"sv, false);
    vc::check("?42"sv, true);
    vc::check("?0.0"sv, false);
    vc::check("?(-42.69)"sv, true);
    vc::check("?_frac(0, 42)"sv, false);
    vc::check("?_frac(1, 42)"sv, true);
    vc::check("?_cplx(0, 0)"sv, false);
    vc::check("?_cplx(0, 1)"sv, true);
    vc::check("?_cplx(1, 0)"sv, true);
    vc::check("f() ; ?f"sv, true);
  }

  TEST(evaluation, t_bitwise)
  {
    vc::check("~2"sv, (~2ll));
    vc::check("~2.0"sv, (~2ll));
    vc::check("~(4/2)"sv, (~2ll));
    vc::check("~2.02"sv);
    vc::check("~(3/2)"sv);

    vc::check("2 & 3"sv, 2ll & 3ll);
    vc::check("2 ^ 3"sv, 2ll ^ 3ll);
    vc::check("2 | 3"sv, 2ll | 3ll);

    vc::check("42 - 40 & 3"sv, 2ll & 3ll);
    vc::check("120 / 60 ^ 3"sv, 2ll ^ 3ll);
    vc::check("2 | 9 / 3"sv, 2ll | 3ll);
  }

  TEST(evaluation, t_comparisons)
  {
    // Eq

    vc::check("2 == 2"sv, true);
    vc::check("2 == 3"sv, false);
    vc::check("2 == 2.0"sv, true);
    vc::check("2 == _cplx(_frac(4, 2))"sv, true);
    vc::check("_true == _cplx(_frac(2, 2))"sv, true);

    // Not eq

    vc::check("2 != 2"sv, false);
    vc::check("2 != 3"sv, true);
    vc::check("2 != 2.0"sv, false);
    vc::check("2 != _cplx(_frac(4, 2))"sv, false);
    vc::check("_true != _cplx(_frac(2, 2))"sv, false);

    // Rel

    vc::check("1 < 2"sv, true);
    vc::check("2 > 1"sv, true);
    vc::check("2 < 2"sv, false);
    vc::check("2 <= 2"sv, true);
    vc::check("2 > 2"sv, false);
    vc::check("2 >= 2"sv, true);


    // Funcs
    
    vc::check("f(); f == f"sv, true);
    vc::check("f1(); f2(); f1 == f2"sv, false);
    vc::check("f(); f < 1"sv);
    vc::check("f1(); f2(); f1 < f2"sv);

    // Weird

    vc::check("2 > 3 < _true"sv, true);
    vc::check("1 == 2 == 0"sv, true);
    vc::check("2 > 3 == _cplx(3, 4) < _frac(10, 6)"sv, true);

  }

  TEST(evaluation, t_arr_eq)
  {
    vc::check("a = [1,2,3] : b = a : a == b"sv, true);
    vc::check("a = [1,2,3] : b = a : a != b"sv, false);
    vc::check("[1,2,3] == [1,2,3]"sv, true);
    vc::check("[1,2,3] != [1,2,3]"sv, false);
    vc::check("[1,2,3] != [1,2,4]"sv, true);
    vc::check("[1,2,3] == [1,2,4]"sv, false);
    vc::check("[1,2] != [1,2,4]"sv, true);
    vc::check("[1,2] == [1,2,4]"sv, false);
    vc::check("[1.0,2.0] == [1,2]"sv, true);
    vc::check("[1.0,2.0] != [1,2]"sv, false);
    vc::check("[1, [2, 3], 4] == [1, [2, 3], 4]"sv, true);
    vc::check("[1, [2, 3], 4] != [1, [2, 3], 4]"sv, false);
  }

  TEST(evaluation, t_arr_less)
  {
    vc::check("a = [1,2,3] : b = a : a < b"sv, false);
    vc::check("[1,2,3] < [1,2,3]"sv, false);
    vc::check("[1,2,3] < [1,2,4]"sv, true);
    vc::check("[1,2,4] < [1,2,3]"sv, false);
    vc::check("[1,2] < [1,2,4]"sv, true);
    vc::check("[1,2,4] < [1,2]"sv, false);
    vc::check("[1.0,2.0] < [1,2]"sv, false);
    vc::check("[1, [2, 3], 4] < [1, [2, 3], 4]"sv, false);
    vc::check("[1, [2, 3], 4, 1] < [1, [2, 3], 4]"sv, false);
    vc::check("[1, [2, 3], 4] < [1, [2, 3, 5], 4]"sv, true);
  }

  TEST(evaluation, t_arr_less_eq)
  {
    vc::check("a = [1,2,3] : b = a : a <= b"sv, true);
    vc::check("[1,2,3] <= [1,2,3]"sv, true);
    vc::check("[1,2,3] <= [1,2,4]"sv, true);
    vc::check("[1,2,4] <= [1,2,3]"sv, false);
    vc::check("[1,2] <= [1,2,4]"sv, true);
    vc::check("[1,2,4] <= [1,2]"sv, false);
    vc::check("[1.0,2.0] <= [1,2]"sv, true);
    vc::check("[1, [2, 3], 4] <= [1, [2, 3], 4]"sv, true);
    vc::check("[1, [2, 3], 4, 1] <= [1, [2, 3], 4]"sv, false);
    vc::check("[1, [2, 3], 4] <= [1, [2, 3, 5], 4]"sv, true);
  }

  TEST(evaluation, t_arr_greater)
  {
    vc::check("a = [1,2,3] : b = a : a > b"sv, false);
    vc::check("[1,2,3] > [1,2,3]"sv, false);
    vc::check("[1,2,3] > [1,2,4]"sv, false);
    vc::check("[1,2,4] > [1,2,3]"sv, true);
    vc::check("[1,2] > [1,2,4]"sv, false);
    vc::check("[1,2,4] > [1,2]"sv, true);
    vc::check("[1.0,2.0] > [1,2]"sv, false);
    vc::check("[1, [2, 3], 4] > [1, [2, 3], 4]"sv, false);
    vc::check("[1, [2, 3], 4, 1] > [1, [2, 3], 4]"sv, true);
    vc::check("[1, [2, 3], 4] > [1, [2, 3, 5], 4]"sv, false);
  }

  TEST(evaluation, t_arr_greater_eq)
  {
    vc::check("a = [1,2,3] : b = a : a >= b"sv, true);
    vc::check("[1,2,3] >= [1,2,3]"sv, true);
    vc::check("[1,2,3] >= [1,2,4]"sv, false);
    vc::check("[1,2,4] >= [1,2,3]"sv, true);
    vc::check("[1,2] >= [1,2,4]"sv, false);
    vc::check("[1,2,4] >= [1,2]"sv, true);
    vc::check("[1.0,2.0] >= [1,2]"sv, true);
    vc::check("[1, [2, 3], 4] >= [1, [2, 3], 4]"sv, true);
    vc::check("[1, [2, 3], 4, 1] >= [1, [2, 3], 4]"sv, true);
    vc::check("[1, [2, 3], 4] >= [1, [2, 3, 5], 4]"sv, false);
  }

  TEST(evaluation, t_short_circuit)
  {
    vc::check("_true  && _true"sv,  true);
    vc::check("_true  && _false"sv, false);
    vc::check("_false && _true"sv,  false);
    vc::check("_false && _false"sv, false);
    vc::check("_true  || _true"sv,  true);
    vc::check("_true  || _false"sv, true);
    vc::check("_false || _true"sv,  true);
    vc::check("_false || _false"sv, false);
  }

  TEST(evaluation, t_binary)
  {
    vc::check("2 * 2"sv, 4ll);
    vc::check("1.5 + 2 * 3"sv, 7.5);
    vc::check("128 - 127 - 1"sv, 0ll);
    vc::check("1.5 * 2 + 6 / 3"sv, 5.0);
    vc::check("-2--3"sv, 1ll);
  }

  TEST(evaluation, t_variable)
  {
    vc::check("a = 10 : a * 2"sv, 20ll);
    vc::check("a = 10 : a = a * a + a"sv, 110ll);
    vc::check("a = 2 : b = a + 2.5 : c = a + b"sv, 6.5);
    vc::check("a = 2 * 3 + 4 + 5"sv, 15ll);
    vc::check("var123 = 2.5 * 3.5 : var123"sv, 8.75);
  }

  TEST(evaluation, t_result)
  {
    vc::check("2 + 2 : _result"sv, 4ll);
    vc::check("2 + 2 : -_result"sv, -4ll);
    vc::check("2 + 2 : _result + 3 : _result"sv, 7ll);
    vc::check("5 * _result"sv, 25ll);
    vc::check("10 : _result * (_result + 2)"sv, 120ll);
  }

  TEST(evaluation, t_func_call)
  {
    vc::check("func(a, b) a * b; func(2, 5)"sv, 10ll);
    vc::check("func(x) x; 1 + func(1) + _result"sv, 4ll);
    vc::check("func(x) in(x) x + 1; x + in(x); func(1)"sv, 3ll);
    vc::check("f() k(a, b) a + b; k; a = f(): a(10, 11)"sv, 21ll);
    vc::check("f() 10; k(x) x() + 5; k(f)"sv, 15ll);
    vc::check("f() k(a) a * 3;; f()(3) + f()(5)"sv, 24ll);
    vc::check("f() _fn(a) a * 3;; f()(3) + f()(5)"sv, 24ll);
  }

  TEST(evaluation, t_overflow)
  {
    auto core = get_tnac(1);
    core.on_semantic_error([](auto&&, auto msg) noexcept
      {
        EXPECT_EQ(msg, "Stack overflow"sv);
      });

    core.evaluate("f() f(); f()"sv);
  }
}