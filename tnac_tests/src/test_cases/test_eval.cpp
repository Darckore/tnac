#include "test_cases/test_common.hpp"
#include "evaluator/evaluator.hpp"

namespace tnac_tests
{
  namespace detail
  {
    namespace
    {
      template <tnac::eval::detail::expr_result T>
      void check_eval(string_t input, T expected) noexcept
      {
        parse_helper p;
        p.parser(input);

        tnac::eval::registry reg;
        tnac::evaluator ev{ reg };
        ev(p.parser.root());

        auto res = reg.evaluation_result();
        tnac::eval::on_value(res, [expected](auto val) noexcept
          {
            if constexpr (tnac::is_same_noquals_v<decltype(val), tnac::eval::invalid_val_t>)
            {
              ASSERT_TRUE(false) << "Undefined value detected";
            }
            else
            {
              using ct = std::common_type_t<decltype(val), decltype(expected)>;
              ASSERT_TRUE(utils::eq(static_cast<ct>(expected), static_cast<ct>(val)));
            }
          });
      }
    }
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