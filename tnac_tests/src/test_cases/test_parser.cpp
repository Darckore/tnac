#include "parser/parser.hpp"
#include "sema/sema.hpp"

namespace tnac_tests
{
  namespace detail
  {
    namespace tree = tnac::ast;
    using tree::node_kind;
    using tnac::string_t;

    struct parse_helper
    {
      parse_helper() :
        parser{ builder, sema }
      {}

      auto operator()(string_t input) noexcept
      {
        return parser(input);
      }

      tnac::ast::builder builder;
      tnac::sema sema;
      tnac::parser parser;
    };

    template <node_kind kind, std::size_t N>
    void check_simple_exprs(const std::array<string_t, N>& inputs)
    {
      parse_helper p;

      for (auto input : inputs)
      {
        auto ast = p(input);
        EXPECT_NE(ast, nullptr) << "Null AST for input: " << input;

        if (ast)
        {
          const auto nodeKind = ast->what();
          EXPECT_EQ(nodeKind, kind) << "Bad kind for input: " << input;
          if (nodeKind == kind)
          {
            auto&& tok = static_cast<tree::expr&>(*ast).pos();
            EXPECT_TRUE(input.starts_with(tok.m_value));
          }
        }
      }
    }
  }

  TEST(parser, t_literals)
  {
    constexpr std::array inputArr{
      "0"sv, "42"sv, "042"sv, "0b1101"sv, "0xfF2"sv, "42.69"sv
    };

    using tnac::ast::node_kind;
    using detail::check_simple_exprs;
    check_simple_exprs<node_kind::Literal>(inputArr);
  }

  TEST(parser, t_unaries)
  {
    constexpr std::array inputArr{
      "+0"sv, "-42"sv, "+042"sv, "+0b1101"sv, "-0xfF2"sv, "-42.69"sv
    };

    using tnac::ast::node_kind;
    using detail::check_simple_exprs;
    check_simple_exprs<node_kind::Unary>(inputArr);
  }

  TEST(parser, t_binaries)
  {
    constexpr std::array inputArr{
      "+0 - 1"sv, "-42 + 0xff"sv, "042 * 2"sv, "0b1101 + -0.5"sv, "0*0xfF2"sv, "-42.69 / 0.0 / 2"sv
    };

    using tnac::ast::node_kind;
    using detail::check_simple_exprs;
    check_simple_exprs<node_kind::Binary>(inputArr);
  }
}