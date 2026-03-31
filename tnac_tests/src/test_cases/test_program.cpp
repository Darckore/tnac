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

#if 0

namespace tnac::tests
{


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

  TEST(program, t_example6)
  {
    array_builder builder;
    constexpr auto op1 = 42ll;
    constexpr auto op2 = 69ll;

    auto&& inner = builder.add(2);
    inner.emplace_back(op2 - op1);
    inner.emplace_back(op2 * op1);

    auto&& arr = builder.add(3);
    arr.emplace_back(op1 + op2);
    arr.emplace_back(builder.to_array_type(inner));
    arr.emplace_back(static_cast<double>(op1) / op2);

    verify_program(TEST_EXAMPLE(6), builder.to_array_type(arr));
  }
}
#endif