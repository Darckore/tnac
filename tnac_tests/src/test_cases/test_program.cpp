#include "test_cases/test_common.hpp"

namespace tnac_tests
{
  #define TEST_EXAMPLE(N) "tests/example"#N".tnac"

  TEST(program, t_example_comments)
  {
    verify_program(TEST_EXAMPLE(_comments), 42ll);
  }

  TEST(program, t_example_fact)
  {
    auto vc = read_program(TEST_EXAMPLE(_fact), true);
    std::array expected{
      std::pair{ "factorial(0)"sv,        1ll },
      std::pair{ "factorial(1)"sv,        1ll },
      std::pair{ "factorial(2)"sv,        2ll },
      std::pair{ "factorial(3)"sv,        6ll },
      std::pair{ "factorial(4)"sv,       24ll },
      std::pair{ "factorial(5)"sv,      120ll },
      std::pair{ "factorial(6)"sv,      720ll },
      std::pair{ "factorial(7)"sv,     5040ll },
      std::pair{ "factorial(8)"sv,    40320ll },
      std::pair{ "factorial(9)"sv,   362880ll },
      std::pair{ "factorial(10)"sv, 3628800ll },
    };

    for (auto&& [input, val] : expected)
    {
      vc(input, val);
    }
  }

  TEST(program, t_example_ret)
  {
    verify_program(TEST_EXAMPLE(_ret), 42ll);
  }

  TEST(program, t_example0)
  {
    verify_program(TEST_EXAMPLE(0), cplx{ 0, 1 });
  }

  TEST(program, t_example1)
  {
    verify_program(TEST_EXAMPLE(1), frac{ 55 });
  }

  TEST(program, t_example2)
  {
    verify_program(TEST_EXAMPLE(2), 15ll);
  }

  TEST(program, t_example3)
  {
    verify_program(TEST_EXAMPLE(3), cplx{});
  }

  TEST(program, t_example4)
  {
    verify_program(TEST_EXAMPLE(4), 42ll + 42ll);
  }

  TEST(program, t_example5)
  {
    verify_program(TEST_EXAMPLE(5), 210ll);
  }
}