#include "test_cases/test_common.hpp"

namespace tnac_tests
{
  namespace detail
  {
    namespace
    {
      using tnac::string_t;
      using tnac::buf_t;

      void read_file(string_t fname, buf_t& buf) noexcept
      {
        fsys::path fn{ fname };

        std::error_code errc;
        fn = fsys::absolute(fn, errc);
        ASSERT_FALSE(static_cast<bool>(errc)) << "Bad file name " << fname;
        std::ifstream in{ fn.string() };
        ASSERT_TRUE(static_cast<bool>(in));
        in.seekg(0, std::ios::end);
        buf.reserve(in.tellg());
        in.seekg(0, std::ios::beg);
        using it = std::istreambuf_iterator<tnac::buf_t::value_type>;
        buf.assign(it{ in }, it{});
      }

      void on_eval_error(const tnac::token& tok, string_t msg) noexcept
      {
        FAIL() << "Eval error " << msg << " at " << tok.m_value;
      }

      void on_parse_error(const tnac::ast::error_expr& err) noexcept
      {
        FAIL() << "Parse error " << err.message() << " at " << err.pos().m_value;
      }

      template <testable_val T>
      void parse_file(string_t fname, T expected) noexcept
      {
        buf_t input;
        read_file(fname, input);
        ASSERT_FALSE(input.empty());

        using tnac::packages::tnac_core;
        tnac_core core{ 100 };

        auto&& ev = core.get_eval();
        auto&& parser = core.get_parser();
        ev.on_error(on_eval_error);
        parser.on_parse_error(on_parse_error);

        parser(input);
        ev(parser.root());
        verify(ev.last_result(), expected);
      }
    }
  }

  #define TEST_EXAMPLE(N) "tests/example"#N".tnac"

  namespace
  {
    using detail::frac;
    using detail::cplx;
    using detail::parse_file;
  }

  TEST(program, t_example_comments)
  {
    parse_file(TEST_EXAMPLE(_comments), 42ll);
  }

  TEST(program, t_example0)
  {
    parse_file(TEST_EXAMPLE(0), cplx{ 0, 1 });
  }

  TEST(program, t_example1)
  {
    parse_file(TEST_EXAMPLE(1), frac{ 55 });
  }

  TEST(program, t_example2)
  {
    parse_file(TEST_EXAMPLE(2), 15ll);
  }

  TEST(program, t_example3)
  {
    parse_file(TEST_EXAMPLE(3), cplx{});
  }

  TEST(program, t_example4)
  {
    parse_file(TEST_EXAMPLE(4), 42ll + 42ll);
  }
}