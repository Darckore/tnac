#include "test_cases/test_common.hpp"

#define TEST_EXAMPLE(N) "tests/example"#N".tnac"sv

namespace tnac::tests
{
  TEST(program, t_playground)
  {
    source_tester st{ TEST_EXAMPLE(1) };
    st.test("example1"sv, 15, 1);
  }

  TEST(program, t_example_comments)
  {
    source_tester st{ TEST_EXAMPLE(_comments) };
    st.test("example_comments"sv, 42);
  }

  TEST(program, t_example_ret)
  {
    source_tester st{ TEST_EXAMPLE(_ret) };
    st.test("example_ret"sv, 42);
  }

  TEST(program, t_example_fact)
  {
    source_tester st{ TEST_EXAMPLE(_fact) };
    constexpr auto fn = "example_fact.factorial"sv;
    st
      .test(fn, 1, 0)
      .test(fn, 1, 1)
      .test(fn, 2, 2)
      .test(fn, 6, 3)
      .test(fn, 24, 4)
      .test(fn, 120, 5)
      .test(fn, 720, 6)
      .test(fn, 5040, 7)
      .test(fn, 40320, 8)
      .test(fn, 362880, 9)
      .test(fn, 3628800, 10)
    ;
  }

  TEST(program, t_example_fib)
  {
    source_tester st{ TEST_EXAMPLE(_fib) };
    constexpr auto fn = "example_fib.fib"sv;
    st
      .test(fn, -1, -1)
      .test(fn, 0, 0)
      .test(fn, 1, 1)
      .test(fn, 1, 2)
      .test(fn, 2, 3)
      .test(fn, 3, 4)
      .test(fn, 5, 5)
      .test(fn, 8, 6)
      .test(fn, 13, 7)
      .test(fn, 21, 8)
      .test(fn, 34, 9)
      .test(fn, 55, 10)
    ;
  }

}
