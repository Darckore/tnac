#include "test_cases/test_common.hpp"
#include "evaluator/evaluator.hpp"

namespace tnac_tests
{
  namespace detail
  {
    namespace
    {
      using cplx = tnac::complex_type;
      using frac = tnac::fraction_type;

      template <tnac::eval::detail::expr_result T>
      bool eq(const T& l, const T& r) noexcept
      {
        return l == r;
      }

      template <>
      bool eq(const tnac::float_type& l, const tnac::float_type& r) noexcept
      {
        if (std::isinf(l) && std::isinf(r))
          return true;

        if (std::isnan(l) && std::isnan(r))
          return true;

        return utils::eq(l, r);
      }

      testing::Message& operator<<(testing::Message& msg, const frac& f) noexcept
      {
        if (f.sign() < 0) msg << '-';
        msg << f.num() << ',' << f.denom();
        return msg;
      }

      auto parse_input(string_t input, tnac::eval::registry& reg) noexcept
      {
        parse_helper p;
        p.parser(input);
        tnac::evaluator ev{ reg };
        ev(p.parser.root());

        return reg.evaluation_result();
      }

      template <tnac::eval::detail::expr_result T>
      void check_eval(string_t input, T expected) noexcept
      {
        tnac::eval::registry reg;
        auto res = parse_input(input, reg);
        tnac::eval::on_value(res, [expected](auto val) noexcept
          {
            if constexpr (tnac::is_same_noquals_v<decltype(val), tnac::eval::invalid_val_t>)
            {
              ASSERT_TRUE(false) << "Undefined value detected";
            }
            else if constexpr (!tnac::is_same_noquals_v<decltype(val), T>)
            {
              ASSERT_TRUE(false) << "Wrong value type";
            }
            else
            {
              ASSERT_TRUE(eq(expected, val)) << "expected: " << expected << " got: " << val;
            }
          });
      }

      void check_invalid(string_t input) noexcept
      {
        tnac::eval::registry reg;
        auto res = parse_input(input, reg);
        ASSERT_TRUE(!res);
      }
    }
  }

  TEST(evaluation, t_basic_add)
  {
    using detail::check_eval;
    using cplx = detail::cplx;
    using frac = detail::frac;

    // Int
    check_eval("1 + 2"sv, 3ll);
    check_eval("1 + 2.0"sv, 3.0);
    check_eval("1 + _complex(1, 2)"sv, cplx{ 2.0, 2.0 });
    check_eval("_complex(1, 2) + 2"sv, cplx{ 3.0, 2.0 });
    check_eval("2 + _fraction(1, 2)"sv, frac{ 5, 2 });
    check_eval("_fraction(1, 2) + 2"sv, frac{ 5, 2 });

    // Float
    check_eval("4.0 + 5.0"sv, 9.0);
    check_eval("4.0 + 5"sv, 9.0);
    check_eval("4.0 + _complex(1.0, 6)"sv, cplx{ 5.0, 6.0 });
    check_eval("_complex(2.0, 6) + 5.0"sv, cplx{ 7.0, 6.0 });
    check_eval("4.0 + _fraction(1, 2)"sv, 4.5);
    check_eval("_fraction(3, 6) + 5.0"sv, 5.5);

    // Complex
    check_eval("_complex(7, 10) + _complex(10, 11)"sv, cplx{ 17.0, 21.0 });
    check_eval("_complex(7, 10) + _fraction(5, 10)"sv, cplx{ 7.5, 10.0 });

    // Fraction
    check_eval("_fraction(1,2) + _fraction(1,3)"sv, frac{ 5, 6 });
  }

  TEST(evaluation, t_basic_sub)
  {
    using detail::check_eval;
    using cplx = detail::cplx;
    using frac = detail::frac;

    // Int
    check_eval("1 - 2"sv, -1ll);
    check_eval("1 - 2.0"sv, -1.0);
    check_eval("1 - _complex(1, 2)"sv, cplx{ 0.0, -2.0 });
    check_eval("_complex(1, 2) - 2"sv, cplx{ -1.0, 2.0 });
    check_eval("2 - _fraction(1, 2)"sv, frac{ 3, 2 });
    check_eval("_fraction(1, 2) - 2"sv, frac{ 3, 2, -1 });

    // Float
    check_eval("4.0 - 5.0"sv, -1.0);
    check_eval("6.0 - 5"sv, 1.0);
    check_eval("4.0 - _complex(1.0, 6)"sv, cplx{ 3.0, -6.0 });
    check_eval("_complex(2.0, 6) - 5.0"sv, cplx{ -3.0, 6.0 });
    check_eval("4.0 - _fraction(1, 2)"sv, 3.5);
    check_eval("_fraction(3, 6) - 5.0"sv, -4.5);

    // Complex
    check_eval("_complex(7, 11) - _complex(10, 11)"sv, cplx{ -3.0, 0.0 });
    check_eval("_complex(7, 10) - _fraction(5, 10)"sv, cplx{ 6.5, 10.0 });

    // Fraction
    check_eval("_fraction(1,2) - _fraction(1,3)"sv, frac{ 1, 6 });
  }

  TEST(evaluation, t_basic_mul)
  {
    using detail::check_eval;
    using cplx = detail::cplx;
    using frac = detail::frac;

    // Int
    check_eval("1 * 2"sv, 2ll);
    check_eval("1 * 2.0"sv, 2.0);
    check_eval("1 * _complex(1, 2)"sv, cplx{ 1.0, 2.0 });
    check_eval("_complex(1, 2) * 2"sv, cplx{ 2.0, 4.0 });
    check_eval("2 * _fraction(1, 2)"sv, frac{ 1, 1 });
    check_eval("_fraction(1, 2) * 2"sv, frac{ 1, 1 });

    // Float
    check_eval("4.0 * 5.0"sv, 20.0);
    check_eval("6.0 * 5"sv, 30.0);
    check_eval("4.0 * _complex(1.0, 6)"sv, cplx{ 4.0, 24.0 });
    check_eval("_complex(2.0, 6) * 5.0"sv, cplx{ 10.0, 30.0 });
    check_eval("4.0 * _fraction(1, 2)"sv, 2.0);
    check_eval("_fraction(3, 6) * 5.0"sv, 2.5);

    // Complex
    check_eval("_complex(7, 11) * _complex(10, 11)"sv, cplx{ -51.0, 187.0 });
    check_eval("_complex(7, 10) * _fraction(5, 10)"sv, cplx{ 3.5, 5.0 });

    // Fraction
    check_eval("_fraction(1,2) * _fraction(1,3)"sv, frac{ 1, 6 });
  }

  TEST(evaluation, t_basic_div)
  {
    using detail::check_eval;
    using cplx = detail::cplx;
    using frac = detail::frac;
    static constexpr auto inf = std::numeric_limits<tnac::float_type>::infinity();

    // Int
    check_eval("2 / 2"sv, 1ll);
    check_eval("1 / 0"sv, inf);
    check_eval("1 / _complex(1, 2)"sv, cplx{ 0.2, -0.4 });
    check_eval("_complex(1, 2) / 2"sv, cplx{ 0.5, 1.0 });
    check_eval("2 / _fraction(1, 2)"sv, frac{ 4, 1 });
    check_eval("_fraction(1, 2) / 2"sv, frac{ 1, 4 });

    // Float
    check_eval("4.0 / 5.0"sv, 0.8);
    check_eval("6.0 / 5"sv, 1.2);
    check_eval("4.0 / _complex(2.0, 4)"sv, cplx{ 0.4, -0.8 });
    check_eval("_complex(2.0, 6) / 5.0"sv, cplx{ 0.4, 1.2 });
    check_eval("4.0 / _fraction(1, 2)"sv, 8.0);
    check_eval("_fraction(3, 6) / 5.0"sv, 0.1);

    // Complex
    check_eval("_complex(-51, 187) / _complex(10, 11)"sv, cplx{ 7.0, 11.0 });
    check_eval("_complex(7, 10) / _fraction(5, 10)"sv, cplx{ 14.0, 20.0 });

    // Fraction
    check_eval("_fraction(1,2) / _fraction(1,3)"sv, frac{ 3, 2 });
  }

  TEST(evaluation, t_basic_mod)
  {
    using detail::check_eval;
    using cplx = detail::cplx;
    using frac = detail::frac;
    static constexpr auto nan = std::numeric_limits<tnac::float_type>::quiet_NaN();

    // Int
    check_eval("2 % 2"sv, 0ll);
    check_eval("1 % 0"sv, nan);
    check_eval("1 % _complex(1, 2)"sv, 0.0);
    check_eval("_complex(1, 2) % 2"sv, cplx{ 1.0, 0.0 });
    check_eval("_fraction(1, 1) % _complex(1, 2)"sv, 0.0);
    check_eval("_complex(1, 2) % _fraction(2, 1)"sv, cplx{ 1.0, 0.0 });

    // Float
    check_eval("4.0 % 5.0"sv, 4.0);
    check_eval("6.0 % 5"sv, 1.0);
    check_eval("4.0 % _complex(2.0, 4)"sv, 0.0);
    check_eval("_complex(2.0, 6) % 5.0"sv, cplx{ 2.0, 1.0 });
    check_eval("_fraction(4, 1) % _complex(2.0, 4)"sv, 0.0);
    check_eval("_complex(2.0, 6) % _fraction(5, 1)"sv, cplx{ 2.0, 1.0 });

    // Complex
    check_eval("_complex(26, 120) % _complex(37, 226)"sv, cplx{ -11.0, -106.0 });

    // Fraction
    check_eval("_fraction(5, 1) % _fraction(3, 1)"sv, 2.0);
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

  TEST(evaluation, t_bitwise)
  {
    using detail::check_eval;
    using detail::check_invalid;

    check_eval("~2", (~2ll));
    check_eval("~(4/2)", (~2ll));
    check_invalid("~2.0");
    check_invalid("~(3/2)");
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
}