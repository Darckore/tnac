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
        auto ast = p.parser(input);

        tnac::eval::registry reg;
        tnac::evaluator ev{ reg };
        ev(ast);

        auto res = reg.evaluation_result();
        tnac::eval::on_value(res, [expected](auto val) noexcept
          {
            if constexpr (tnac::is_same_noquals_v<decltype(val), tnac::eval::invalid_val_t>)
            {
              ASSERT_TRUE(false) << "Undefined value detected";
            }
            else
            {
              ASSERT_EQ(expected, val);
            }
          });
      }
    }
  }

  TEST(evaluation, t_literal)
  {
    using detail::check_eval;
    check_eval("2", 2ll);
    check_eval("0b101", 5ll);
    check_eval("010", 8ll);
    check_eval("0xff", 255ll);
    check_eval("42.69", 42.69);
  }
}