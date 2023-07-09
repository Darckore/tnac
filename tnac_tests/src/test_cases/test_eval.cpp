#include "test_cases/test_common.hpp"
#include "evaluator/evaluator.hpp"

namespace tnac_tests
{
  TEST(evaluation, t_basic_add)
  {
    using detail::check_eval;
    using cplx = detail::cplx;
    using frac = detail::frac;

    // Int
    check_eval("_true + 1"sv, 2ll);
    check_eval("1 + 2"sv, 3ll);
    check_eval("1 + 2.0"sv, 3.0);
    check_eval("1 + _cplx(1, 2)"sv, cplx{ 2.0, 2.0 });
    check_eval("_cplx(1, 2) + 2"sv, cplx{ 3.0, 2.0 });
    check_eval("2 + _frac(1, 2)"sv, frac{ 5, 2 });
    check_eval("_frac(1, 2) + 2"sv, frac{ 5, 2 });

    // Float
    check_eval("_true + 41.0"sv, 42.0);
    check_eval("4.0 + 5.0"sv, 9.0);
    check_eval("4.0 + 5"sv, 9.0);
    check_eval("4.0 + _cplx(1.0, 6)"sv, cplx{ 5.0, 6.0 });
    check_eval("_cplx(2.0, 6) + 5.0"sv, cplx{ 7.0, 6.0 });
    check_eval("4.0 + _frac(1, 2)"sv, 4.5);
    check_eval("_frac(3, 6) + 5.0"sv, 5.5);

    // Complex
    check_eval("_true + _cplx(4, 4)"sv, cplx{ 5.0, 4.0 });
    check_eval("_cplx(7, 10) + _cplx(10, 11)"sv, cplx{ 17.0, 21.0 });
    check_eval("_cplx(7, 10) + _frac(5, 10)"sv, cplx{ 7.5, 10.0 });

    // Fraction
    check_eval("_frac(1,2) + _frac(1,3)"sv, frac{ 5, 6 });
  }

  TEST(evaluation, t_basic_sub)
  {
    using detail::check_eval;
    using cplx = detail::cplx;
    using frac = detail::frac;

    // Int
    check_eval("_false - 1"sv, -1ll);
    check_eval("1 - 2"sv, -1ll);
    check_eval("1 - 2.0"sv, -1.0);
    check_eval("1 - _cplx(1, 2)"sv, cplx{ 0.0, -2.0 });
    check_eval("_cplx(1, 2) - 2"sv, cplx{ -1.0, 2.0 });
    check_eval("2 - _frac(1, 2)"sv, frac{ 3, 2 });
    check_eval("_frac(1, 2) - 2"sv, frac{ 3, 2, -1 });

    // Float
    check_eval("42.0 - _true"sv, 41.0);
    check_eval("4.0 - 5.0"sv, -1.0);
    check_eval("6.0 - 5"sv, 1.0);
    check_eval("4.0 - _cplx(1.0, 6)"sv, cplx{ 3.0, -6.0 });
    check_eval("_cplx(2.0, 6) - 5.0"sv, cplx{ -3.0, 6.0 });
    check_eval("4.0 - _frac(1, 2)"sv, 3.5);
    check_eval("_frac(3, 6) - 5.0"sv, -4.5);

    // Complex
    check_eval("_cplx(7, 10) - _true"sv, cplx{ 6.0, 10.0 });
    check_eval("_cplx(7, 11) - _cplx(10, 11)"sv, cplx{ -3.0, 0.0 });
    check_eval("_cplx(7, 10) - _frac(5, 10)"sv, cplx{ 6.5, 10.0 });

    // Fraction
    check_eval("_frac(1,2) - _frac(1,3)"sv, frac{ 1, 6 });
  }

  TEST(evaluation, t_basic_mul)
  {
    using detail::check_eval;
    using cplx = detail::cplx;
    using frac = detail::frac;

    // Int
    check_eval("1 * 2"sv, 2ll);
    check_eval("1 * 2.0"sv, 2.0);
    check_eval("1 * _cplx(1, 2)"sv, cplx{ 1.0, 2.0 });
    check_eval("_cplx(1, 2) * 2"sv, cplx{ 2.0, 4.0 });
    check_eval("2 * _frac(1, 2)"sv, frac{ 1, 1 });
    check_eval("_frac(1, 2) * 2"sv, frac{ 1, 1 });

    // Float
    check_eval("4.0 * 5.0"sv, 20.0);
    check_eval("6.0 * 5"sv, 30.0);
    check_eval("4.0 * _cplx(1.0, 6)"sv, cplx{ 4.0, 24.0 });
    check_eval("_cplx(2.0, 6) * 5.0"sv, cplx{ 10.0, 30.0 });
    check_eval("4.0 * _frac(1, 2)"sv, 2.0);
    check_eval("_frac(3, 6) * 5.0"sv, 2.5);

    // Complex
    check_eval("_cplx(7, 11) * _cplx(10, 11)"sv, cplx{ -51.0, 187.0 });
    check_eval("_cplx(7, 10) * _frac(5, 10)"sv, cplx{ 3.5, 5.0 });

    // Fraction
    check_eval("_frac(1,2) * _frac(1,3)"sv, frac{ 1, 6 });
  }

  TEST(evaluation, t_basic_div)
  {
    using detail::check_eval;
    using cplx = detail::cplx;
    using frac = detail::frac;
    static constexpr auto inf = std::numeric_limits<tnac::eval::float_type>::infinity();

    // Int
    check_eval("2 / 2"sv, 1.0);
    check_eval("1 / 0"sv, inf);
    check_eval("1 / _cplx(1, 2)"sv, cplx{ 0.2, -0.4 });
    check_eval("_cplx(1, 2) / 2"sv, cplx{ 0.5, 1.0 });
    check_eval("2 / _frac(1, 2)"sv, frac{ 4, 1 });
    check_eval("_frac(1, 2) / 2"sv, frac{ 1, 4 });

    // Float
    check_eval("4.0 / 5.0"sv, 0.8);
    check_eval("6.0 / 5"sv, 1.2);
    check_eval("4.0 / _cplx(2.0, 4)"sv, cplx{ 0.4, -0.8 });
    check_eval("_cplx(2.0, 6) / 5.0"sv, cplx{ 0.4, 1.2 });
    check_eval("4.0 / _frac(1, 2)"sv, 8.0);
    check_eval("_frac(3, 6) / 5.0"sv, 0.1);

    // Complex
    check_eval("_cplx(-51, 187) / _cplx(10, 11)"sv, cplx{ 7.0, 11.0 });
    check_eval("_cplx(7, 10) / _frac(5, 10)"sv, cplx{ 14.0, 20.0 });

    // Fraction
    check_eval("_frac(1,2) / _frac(1,3)"sv, frac{ 3, 2 });
  }

  TEST(evaluation, t_basic_mod)
  {
    using detail::check_eval;
    using cplx = detail::cplx;
    using frac = detail::frac;
    static constexpr auto nan = std::numeric_limits<tnac::eval::float_type>::quiet_NaN();

    // Int
    check_eval("2 % 2"sv, 0.0);
    check_eval("1 % 0"sv, nan);
    check_eval("1 % _cplx(1, 2)"sv, cplx{ 1.0, 0.0 });
    check_eval("_cplx(1, 2) % 2"sv, cplx{ -1.0, 0.0 });
    check_eval("_frac(1, 1) % _cplx(1, 2)"sv, cplx{ 1.0, 0.0 });
    check_eval("_cplx(1, 2) % _frac(2, 1)"sv, cplx{ -1.0, 0.0 });

    // Float
    check_eval("4.0 % 5.0"sv, 4.0);
    check_eval("6.0 % 5"sv, 1.0);
    check_eval("4.0 % _cplx(2.0, 4)"sv, cplx{ 0.0, 2.0 });
    check_eval("_cplx(2.0, 6) % 5.0"sv, cplx{ 2.0, 1.0 });
    check_eval("_frac(4, 1) % _cplx(2.0, 4)"sv, cplx{ 0.0, 2.0 });
    check_eval("_cplx(2.0, 6) % _frac(5, 1)"sv, cplx{ 2.0, 1.0 });

    // Complex
    check_eval("_cplx(26, 120) % _cplx(37, 226)"sv, cplx{ -11.0, -106.0 });

    // Fraction
    check_eval("_frac(5, 1) % _frac(3, 1)"sv, 2.0);
  }

  TEST(evaluation, t_basic_pow)
  {
    using detail::check_eval;
    using cplx = detail::cplx;
    using frac = detail::frac;

    check_eval("2 ** 2"sv, 4.0);
    check_eval("1 ** 0"sv, 1.0);
    check_eval("1 ** _cplx(1, 2)"sv, cplx{ 1.0, 0.0 });
    check_eval("_frac(1, 1) ** _cplx(1, 2)"sv, cplx{ 1.0, 0.0 });
    check_eval("4.0 ** 5.0"sv, 1024.0);
    check_eval("6.0 ** 5"sv, 7776.0);
    check_eval("_frac(8, 2) ** _frac(1, 2)"sv, 2.0);

    check_eval("-1 ** 0.5"sv, cplx{ 0.0, 1.0 });
    check_eval("-4 // 2"sv,   cplx{ 0.0, 2.0 });
    check_eval("-4 // -2"sv,  cplx{ 0.0, 0.5 });
  }

  TEST(evaluation, t_literal)
  {
    using detail::check_eval;
    check_eval("2"sv, 2ll);
    check_eval("0b101"sv, 5ll);
    check_eval("010"sv, 8ll);
    check_eval("0xff"sv, 255ll);
    check_eval("42.69"sv, 42.69);
  }

  TEST(evaluation, t_unary)
  {
    using detail::check_eval;
    check_eval("-2"sv, -2ll);
    check_eval("+42.69"sv, 42.69);
    check_eval("-(2 + 3)"sv, -5ll);
    check_eval("-(2/4)"sv, -0.5);
    check_eval("a = 10 : -a"sv, -10ll);
  }

  TEST(evaluation, t_absolute)
  {
    using detail::check_eval;
    using detail::check_invalid;
    check_eval("| _true |"sv, 1ll);
    check_eval("| _false |"sv, 0ll);
    check_eval("| 2 |"sv, 2ll);
    check_eval("| -2 |"sv, 2ll);
    check_eval("| 2.0 |"sv, 2.0);
    check_eval("| -2.0 |"sv, 2.0);
    check_eval("| -2.0 |"sv, 2.0);
    check_eval("| _frac(1, 2) |"sv, detail::frac{1, 2});
    check_eval("| _frac(-1, 2) |"sv, detail::frac{1, 2});
    check_eval("| _cplx(3, 4) |"sv, 5.0);
    
    check_invalid("| _fn(); + 1 |"sv);
  }

  TEST(evaluation, t_log_not)
  {
    using detail::check_eval;
    check_eval("!_false"sv, true);
    check_eval("!_true"sv,  false);
    check_eval("!0"sv,  true);
    check_eval("!42"sv, false);
    check_eval("!0.0"sv, true);
    check_eval("!(-42.69)"sv, false);
    check_eval("!_frac(0, 42)"sv, true);
    check_eval("!_frac(1, 42)"sv, false);
    check_eval("!_cplx(0, 0)"sv, true);
    check_eval("!_cplx(0, 1)"sv, false);
    check_eval("!_cplx(1, 0)"sv, false);
    check_eval("f() ; !f"sv, false);
  }

  TEST(evaluation, t_log_is)
  {
    using detail::check_eval;
    check_eval("?_false"sv, false);
    check_eval("?_true"sv, true);
    check_eval("?0"sv, false);
    check_eval("?42"sv, true);
    check_eval("?0.0"sv, false);
    check_eval("?(-42.69)"sv, true);
    check_eval("?_frac(0, 42)"sv, false);
    check_eval("?_frac(1, 42)"sv, true);
    check_eval("?_cplx(0, 0)"sv, false);
    check_eval("?_cplx(0, 1)"sv, true);
    check_eval("?_cplx(1, 0)"sv, true);
    check_eval("f() ; ?f"sv, true);
  }

  TEST(evaluation, t_bitwise)
  {
    using detail::check_eval;
    using detail::check_invalid;

    check_eval("~2"sv, (~2ll));
    check_eval("~2.0"sv, (~2ll));
    check_eval("~(4/2)"sv, (~2ll));
    check_invalid("~2.02"sv);
    check_invalid("~(3/2)"sv);

    check_eval("2 & 3"sv, 2ll & 3ll);
    check_eval("2 ^ 3"sv, 2ll ^ 3ll);
    check_eval("2 | 3"sv, 2ll | 3ll);

    check_eval("42 - 40 & 3"sv, 2ll & 3ll);
    check_eval("120 / 60 ^ 3"sv, 2ll ^ 3ll);
    check_eval("2 | 9 / 3"sv, 2ll | 3ll);
  }

  TEST(evaluation, t_comparisons)
  {
    using detail::check_eval;
    using detail::check_invalid;

    // Eq

    check_eval("2 == 2"sv, true);
    check_eval("2 == 3"sv, false);
    check_eval("2 == 2.0"sv, true);
    check_eval("2 == _cplx(_frac(4, 2))"sv, true);
    check_eval("_true == _cplx(_frac(2, 2))"sv, true);

    // Not eq

    check_eval("2 != 2"sv, false);
    check_eval("2 != 3"sv, true);
    check_eval("2 != 2.0"sv, false);
    check_eval("2 != _cplx(_frac(4, 2))"sv, false);
    check_eval("_true != _cplx(_frac(2, 2))"sv, false);

    // Rel

    check_eval("1 < 2"sv, true);
    check_eval("2 > 1"sv, true);
    check_eval("2 < 2"sv, false);
    check_eval("2 <= 2"sv, true);
    check_eval("2 > 2"sv, false);
    check_eval("2 >= 2"sv, true);


    // Funcs
    
    check_eval("f(); f == f"sv, true);
    check_eval("f1(); f2(); f1 == f2"sv, false);
    check_invalid("f(); f < 1"sv);
    check_invalid("f1(); f2(); f1 < f2"sv);

    // Weird

    check_eval("2 > 3 < _true"sv, true);
    check_eval("1 == 2 == 0"sv, true);
    check_eval("2 > 3 == _cplx(3, 4) < _frac(10, 6)"sv, true);

  }

  TEST(evaluation, t_short_circuit)
  {
    tnac::packages::tnac_core core{ 0 };

    static constexpr std::array samples {
      "_true  && _true"sv,
      "_true  && _false"sv,
      "_false && _true"sv,
      "_false && _false"sv,
      "_true  || _true"sv,
      "_true  || _false"sv,
      "_false || _true"sv,
      "_false || _false"sv
    };

    auto evaluate = [&](auto idx) noexcept
    {
      auto cur = core.get_parser()(samples[idx]);
      core.get_eval()(cur);
      return utils::try_cast<tnac::ast::binary_expr>(cur);
    };

    using val = std::optional<bool>;
    auto idx = 0u;
    auto check = [&idx](auto expr, val overall, val l, val r) noexcept
    {
      auto exprVal = detail::to_bool(expr->value());
      EXPECT_EQ(exprVal, overall) << "Sample " << idx << " entire";
      auto lVal    = detail::to_bool(expr->left().value());
      EXPECT_EQ(lVal, l) << "Sample " << idx << " left";
      auto rVal    = detail::to_bool(expr->right().value());
      EXPECT_EQ(rVal, r) << "Sample " << idx << " right";
      ++idx;
    };

    // _true  && _true
    auto curNode = evaluate(0); check(curNode, true, true, true);
    // _true  && _false
    curNode = evaluate(1); check(curNode, false, true, false);
    // _false && _true
    curNode = evaluate(2); check(curNode, false, false, {});
    // _false && _false
    curNode = evaluate(3); check(curNode, false, false, {});
    // _true  || _true
    curNode = evaluate(4); check(curNode, true, true, {});
    // _true  || _false
    curNode = evaluate(5); check(curNode, true, true, {});
    // _false || _true
    curNode = evaluate(6); check(curNode, true, false, true);
    // _false || _false
    curNode = evaluate(7); check(curNode, false, false, false);
  }

  TEST(evaluation, t_binary)
  {
    using detail::check_eval;
    check_eval("2 * 2"sv, 4ll);
    check_eval("1.5 + 2 * 3"sv, 7.5);
    check_eval("128 - 127 - 1"sv, 0ll);
    check_eval("1.5 * 2 + 6 / 3"sv, 5.0);
    check_eval("-2--3"sv, 1ll);
  }

  TEST(evaluation, t_variable)
  {
    using detail::check_eval;
    check_eval("a = 10 : a * 2"sv, 20ll);
    check_eval("a = 10 : a = a * a + a"sv, 110ll);
    check_eval("a = 2 : b = a + 2.5 : c = a + b"sv, 6.5);
    check_eval("a = 2 * 3 + 4 + 5"sv, 15ll);
    check_eval("var123 = 2.5 * 3.5 : var123"sv, 8.75);
  }

  TEST(evaluation, t_result)
  {
    using detail::check_eval;
    check_eval("2 + 2 : _result"sv, 4ll);
    check_eval("2 + 2 : -_result"sv, -4ll);
    check_eval("2 + 2 : _result + 3 : _result"sv, 7ll);
    check_eval("5 * _result"sv, 25ll);
    check_eval("10 : _result * (_result + 2)"sv, 120ll);
  }

  TEST(evaluation, t_func_call)
  {
    using detail::check_eval;
    check_eval("func(a, b) a * b; func(2, 5)"sv, 10ll);
    check_eval("func(x) x; 1 + func(1) + _result"sv, 4ll);
    check_eval("func(x) in(x) x + 1; x + in(x); func(1)"sv, 3ll);
    check_eval("f() k(a, b) a + b; k; a = f(): a(10, 11)"sv, 21ll);
    check_eval("f() 10; k(x) x() + 5; k(f)"sv, 15ll);
    check_eval("f() k(a) a * 3;; f()(3) + f()(5)"sv, 24ll);
    check_eval("f() _fn(a) a * 3;; f()(3) + f()(5)"sv, 24ll);
  }

  TEST(evaluation, t_overflow)
  {
    using detail::parse_helper;
    parse_helper ph;
    tnac::eval::registry reg;
    tnac::eval::call_stack cs{ 1 };
    tnac::evaluator ev{ reg, cs };
    ev.on_error([](auto&&, auto msg) noexcept
      {
        EXPECT_EQ(msg, "Stack overflow"sv);
      });

    ph.parser("f() f(); f()"sv);
    ev(ph.parser.root());
  }
}