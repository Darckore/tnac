#include "parser/parser.hpp"

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
        builder{}, parser{ builder }
      {}

      tnac::ast::builder builder;
      tnac::parser parser;
    };

    template <node_kind kind, std::size_t N>
    void check_simple_exprs(const std::array<string_t, N>& inputs)
    {
      parse_helper p;

      for (auto input : inputs)
      {
        auto ast = p.parser(input);
        EXPECT_NE(ast, nullptr) << "Null AST for input: " << input;

        if (ast)
        {
          const auto nodeKind = ast->what();
          EXPECT_EQ(nodeKind, kind) << "Bad kind for input: " << input;
          if (nodeKind == kind)
          {
            auto&& tok = static_cast<tree::expr&>(*ast).pos();
            EXPECT_EQ(tok.m_value, input);
          }
        }
      }
    }
  }

  TEST(parser, t_literals)
  {
    detail::parse_helper p;

    constexpr std::array inputArr{
      "0"sv, "42"sv, "042"sv, "0b1101"sv, "0xfF2"sv
    };

    using tnac::ast::node_kind;
    using detail::check_simple_exprs;
    check_simple_exprs<node_kind::Literal>(inputArr);
  }
}