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
    verify_program(TEST_EXAMPLE(_fact), 3628800ll);
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