#include "test_cases/test_common.hpp"

#define TEST_EXAMPLE(N) "tests/example"#N".tnac"

namespace tnac::tests
{
  TEST(program, t_playground)
  {
    feedback fb;
    auto core = get_tnac(fb);
    core.process_file(TEST_EXAMPLE(1));
    core.compile();
    auto&& cfg = core.get_cfg();
    auto it = cfg.begin();
    ASSERT_NE(it, cfg.end());

    auto mod = *it;
    auto&& ev = core.ir_evaluator();
    ev.enter(*mod);
    ev.evaluate_current();
    utils::unused(ev);
  }
}

#if 0

namespace tnac::tests
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

  TEST(program, t_example_fib)
  {
    auto vc = read_program(TEST_EXAMPLE(_fib), true);
    std::array expected{
      std::pair{ "fib(-1)"sv, -1ll },
      std::pair{ "fib(0)"sv,   0ll },
      std::pair{ "fib(1)"sv,   1ll },
      std::pair{ "fib(2)"sv,   1ll },
      std::pair{ "fib(3)"sv,   2ll },
      std::pair{ "fib(4)"sv,   3ll },
      std::pair{ "fib(5)"sv,   5ll },
      std::pair{ "fib(6)"sv,   8ll },
      std::pair{ "fib(7)"sv,  13ll },
      std::pair{ "fib(8)"sv,  21ll },
      std::pair{ "fib(9)"sv,  34ll },
      std::pair{ "fib(10)"sv, 55ll },
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