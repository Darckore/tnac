#include "test_cases/test_common.hpp"

namespace tnac::tests
{
  using eval::val_ops;

  TEST(evaluation, t_basic_add)
  {
    value_checker{ val_ops::Addition }

      // Int

      .with(true).act(1).verify(2)                        // _true + 1
      .with(1).act(2).verify(3)                           // 1 + 2
      .with(1).act(2.0).verify(3.0)                       // 1 + 2.0
      .with(1).act(cplx{ 1, 2 }).verify(cplx{ 2, 2 })     // 1 + _cplx(1, 2)
      .with(cplx{ 1, 2 }).act(2).verify(cplx{ 3, 2 })     // _cplx(1, 2) + 2
      .with(2).act(frac{ 1, 2 }).verify(frac{ 5, 2 })     // 2 + _frac(1, 2)
      .with(frac{ 1, 2 }).act(2).verify(frac{ 5, 2 })     // _frac(1, 2) + 2

      // Float

      .with(true).act(41.0).verify(42.0)                 //_true + 41.0
      .with(4.0).act(5.0).verify(9.0)                    //4.0 + 5.0
      .with(4.0).act(5).verify(9.0)                      //4.0 + 5
      .with(4.0).act(cplx{ 1, 6 }).verify(cplx{ 5, 6 })  //4.0 + _cplx(1.0, 6)
      .with(cplx{ 2, 6 }).act(5.0).verify(cplx{ 7, 6 })  //_cplx(2.0, 6) + 5.0
      .with(4.0).act(frac{ 1, 2 }).verify(4.5)           //4.0 + _frac(1, 2)
      .with(frac{ 3, 6 }).act(5.0).verify(5.5)           //_frac(3, 6) + 5.0

      // Complex

      .with(true).act(cplx{ 4, 4 }).verify(cplx{ 5, 4 })               //_true + _cplx(4, 4)
      .with(cplx{ 7, 10 }).act(cplx{ 10, 11 }).verify(cplx{ 17, 21 })  //_cplx(7, 10) + _cplx(10, 11)
      .with(cplx{ 7, 10 }).act(frac{ 5, 10 }).verify(cplx{ 7.5, 10 })  //_cplx(7, 10) + _frac(5, 10)

      // Fraction

      .with(frac{ 1, 2 }).act(frac{ 1, 3 }).verify(frac{ 5, 6 })  //_frac(1,2) + _frac(1,3)
      ;
  }

  TEST(evaluation, t_basic_sub)
  {
    value_checker{ val_ops::Subtraction }

      // Int

      .with(false).act(1).verify(-1)                      // _false - 1
      .with(1).act(2).verify(-1)                          // 1 - 2
      .with(1).act(2.0).verify(-1.0)                      // 1 - 2.0
      .with(1).act(cplx{ 1, 2 }).verify(cplx{ 0, -2 })    // 1 - _cplx(1, 2)
      .with(cplx{ 1, 2 }).act(2).verify(cplx{ -1, 2 })    // _cplx(1, 2) - 2
      .with(2).act(frac{ 1, 2 }).verify(frac{ 3, 2 })     // 2 - _frac(1, 2)
      .with(frac{ 1, 2 }).act(2).verify(frac{ 3, 2, -1 }) // _frac(1, 2) - 2

      // Float

      .with(42.0).act(true).verify(41.0)                  // 42.0 - _true
      .with(4.0).act(5.0).verify(-1.0)                    // 4.0 - 5.0
      .with(6.0).act(5).verify(1.0)                       // 6.0 - 5
      .with(4.0).act(cplx{ 1, 6 }).verify(cplx{ 3, -6 })  // 4.0 - _cplx(1.0, 6)
      .with(cplx{ 2, 6 }).act(5.0).verify(cplx{ -3, 6 })  // _cplx(2.0, 6) - 5.0
      .with(4.0).act(frac{ 1, 2 }).verify(3.5)            // 4.0 - _frac(1, 2)
      .with(frac{ 3, 6 }).act(5.0).verify(-4.5)           // _frac(3, 6) - 5.0

      // Complex
      .with(cplx{ 7, 10 }).act(true).verify(cplx{ 6, 10 })             // _cplx(7, 10) - _true
      .with(cplx{ 7, 11 }).act(cplx{ 10, 11 }).verify(cplx{ -3, 0 })   // _cplx(7, 11) - _cplx(10, 11)
      .with(cplx{ 7, 10 }).act(frac{ 5, 10 }).verify(cplx{ 6.5, 10 })  // _cplx(7, 10) - _frac(5, 10)

      // Fraction
      .with(frac{ 1, 2 }).act(frac{ 1, 3 }).verify(frac{ 1, 6 })       //_frac(1,2) - _frac(1,3)
      ;
  }

  TEST(evaluation, t_basic_mul)
  {
    value_checker{ val_ops::Multiplication }
    
      // Int

      .with(1).act(2).verify(2)                        // 1 * 2
      .with(1).act(2.0).verify(2.0)                    // 1 * 2.0
      .with(1).act(cplx{ 1, 2 }).verify(cplx{ 1, 2 })  // 1 * _cplx(1, 2)
      .with(cplx{ 1, 2 }).act(2).verify(cplx{ 2, 4 })  // _cplx(1, 2) * 2
      .with(2).act(frac{ 1, 2 }).verify(frac{ 1, 1 })  // 2 * _frac(1, 2)
      .with(frac{ 1, 2 }).act(2).verify(frac{ 1, 1 })  // _frac(1, 2) * 2

      // Float

      .with(4.0).act(5.0).verify(20.0)                     // 4.0 * 5.0
      .with(6.0).act(5).verify(30.0)                       // 6.0 * 5
      .with(4.0).act(cplx{ 1, 6 }).verify(cplx{ 4, 24 })   // 4.0 * _cplx(1.0, 6)
      .with(cplx{ 2, 6 }).act(5.0).verify(cplx{ 10, 30 })  // _cplx(2.0, 6) * 5.0
      .with(4.0).act(frac{ 1, 2 }).verify(2.0)             // 4.0 * _frac(1, 2)
      .with(frac{ 3, 6 }).act(5.0).verify(2.5)             // _frac(3, 6) * 5.0

       // Complex
       
      .with(cplx{ 7, 11 }).act(cplx{ 10, 11 }).verify(cplx{ -51, 187 })  // _cplx(7, 11) * _cplx(10, 11)
      .with(cplx{ 7, 10 }).act(frac{ 5, 10 }).verify(cplx{ 3.5, 5 })     // _cplx(7, 10) * _frac(5, 10)

      // Fraction
      
      .with(frac{ 1, 2 }).act(frac{ 1, 3 }).verify(frac{ 1, 6 })   // _frac(1,2) * _frac(1,3)
    ;
  }
}

#if 0

namespace tnac::tests
{

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
    vc::check("-4 // -2"sv,  cplx{ 0.0, -0.5 });

    vc::check("-1 ** (1/3)"sv, -1.0);
    vc::check("-8 ** (1/3)"sv, -2.0);
    vc::check("-8 // 3"sv, -2.0);
    vc::check("-8 // -3"sv, -0.5);
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
    vc::check("| _frac(1, 2) |"sv, frac{1, 2});
    vc::check("| _frac(-1, 2) |"sv, frac{1, 2});
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

  TEST(evaluation, t_plain_arr)
  {
    array_builder builder;
    auto&& arr = builder.add(5);
    arr.emplace_back(1ll);
    arr.emplace_back(42ll);
    arr.emplace_back(2.3);
    arr.emplace_back(cplx{ 3, 4 });
    arr.emplace_back(frac{ 1, 2 });
    
    vc::check("[1, 42, 2.3, 3 + 4*_i, _frac(1,2)]"sv, builder.to_array_type(arr));
  }

  TEST(evaluation, t_complex_arr)
  {
    array_builder builder;
    
    auto&& inner1 = builder.add(3);
    inner1.emplace_back(1ll);
    inner1.emplace_back(2ll);
    inner1.emplace_back(3ll);

    auto&& inner2 = builder.add(2);
    inner2.emplace_back(4.0);
    inner2.emplace_back(cplx{ 1, 2 });

    auto&& outer = builder.add(3);
    outer.emplace_back(7ll);
    outer.emplace_back(builder.to_array_type(inner1));
    outer.emplace_back(builder.to_array_type(inner2));

    vc::check("[7, [1, 2, 3], [4.0, 1 + 2*_i]]"sv, builder.to_array_type(outer));
  }

  TEST(evaluation, t_arr_unary)
  {
    array_builder builder;
    auto&& arr = builder.add(2);
    arr.emplace_back(-42ll);
    arr.emplace_back(-69.0);

    vc::check("-[42, 69.0]"sv, builder.to_array_type(arr));
  }

  TEST(evaluation, t_arr_binary_scalar)
  {
    array_builder builder;
    auto&& arr = builder.add(2);
    arr.emplace_back(42ll);
    arr.emplace_back(69.0);

    vc::check("[40, 67.0] + 2"sv, builder.to_array_type(arr));
  }

  TEST(evaluation, t_arr_binary_arr)
  {
    array_builder builder;
    auto&& arr = builder.add(4);
    arr.emplace_back(4.0);
    arr.emplace_back(8.0);
    arr.emplace_back(9.0);
    arr.emplace_back(27.0);

    vc::check("[2, 3] ** [2, 3]"sv, builder.to_array_type(arr));
  }

  TEST(evaluation, t_arr_abs)
  {
    array_builder builder;
    auto&& arr = builder.add(2);
    arr.emplace_back(42ll);
    arr.emplace_back(5.0);

    vc::check("|[-42, 3 + 4*_i]|"sv, builder.to_array_type(arr));
  }

  TEST(evaluation, t_arr_call)
  {
    array_builder builder;
    auto&& arr = builder.add(2);
    arr.emplace_back(12ll);
    arr.emplace_back(20ll);

    vc::check("[ _fn(x) x + 2;, _fn(x) x * 2; ](10)"sv, builder.to_array_type(arr));
  }

  TEST(evaluation, t_arr_complex_call)
  {
    array_builder builder;
    
    auto&& inner = builder.add(2);
    inner.emplace_back(20ll);
    inner.emplace_back(8ll);

    auto&& arr = builder.add(2);
    arr.emplace_back(12ll);
    arr.emplace_back(builder.to_array_type(inner));

    vc::check("[ _fn(x) x + 2;, [ _fn(x) x * 2;, _fn(x) x - 2; ] ](10)"sv, builder.to_array_type(arr));
  }

}
#endif