#include "test_cases/test_common.hpp"
#include "ast/ast_visitor.hpp"

namespace tnac_tests
{
  namespace detail
  {
    namespace
    {
      struct expected_node
      {
        string_t data{};
        node_kind kind{};
        node_kind parent{};
        bool nullParent{};
      };

      class tree_checker : public tree::const_bottom_up_visitor<tree_checker>
      {
      public:
        using test_data = std::span<expected_node>;
        using data_iter = test_data::iterator;

      public:
        CLASS_SPECIALS_NODEFAULT(tree_checker);

        tree_checker(test_data expected) noexcept :
          m_data{ expected },
          m_iter{ expected.begin() }
        {}

      public:
        void visit(const tree::scope& scope) noexcept
        {
          check_node(scope, "");
        }

        void visit(const tree::assign_expr& expr) noexcept
        {
          check_node(expr, expr.op().m_value);
        }

        void visit(const tree::decl_expr& expr) noexcept
        {
          check_node(expr, "");
        }

        void visit(const tree::var_decl& decl) noexcept
        {
          check_node(decl, decl.name());
        }

        void visit(const tree::binary_expr& expr) noexcept
        {
          check_node(expr, expr.op().m_value);
        }

        void visit(const tree::unary_expr& expr) noexcept
        {
          check_node(expr, expr.op().m_value);
        }

        void visit(const tree::paren_expr& expr) noexcept
        {
          check_node(expr, "");
        }

        void visit(const tree::lit_expr& expr) noexcept
        {
          check_node(expr, expr.pos().m_value);
        }

        void visit(const tree::id_expr& expr) noexcept
        {
          check_node(expr, expr.name());
        }

        void visit(const tree::error_expr& expr) noexcept
        {
          check_node(expr, expr.message());
        }

      private:
        void check_node(const tree::node& node, string_t nodeStr) noexcept
        {
          ASSERT_NE(m_iter, m_data.end()) << "Unexpected end of data";
          auto&& expected = *m_iter;
          ++m_iter;

          EXPECT_EQ(nodeStr, expected.data);

          const auto expKind = node.what();
          EXPECT_EQ(expKind, expected.kind) << "Wrong kind at node " << nodeStr;

          auto parent = node.parent();
          if (!parent)
          {
            EXPECT_TRUE(expected.nullParent) << "Parent was null at node " << nodeStr;
          }
          else
          {
            EXPECT_EQ(parent->what(), expected.parent) << "Wrong parent at node " << nodeStr;
          }
        }

      private:
        test_data m_data{};
        data_iter m_iter{};
      };

      struct error_checker
      {
        error_checker(parse_helper& ph, tnac::string_t msg) noexcept :
          expectedErr{ msg }
        {
          ph.parser.on_error([this](auto&& err) noexcept { on_error(err); });
        }

        void on_error(const tree::error_expr& err) noexcept
        {
          ASSERT_EQ(err.message(), expectedErr);
        }

        string_t expectedErr{};
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

      void check_tree_structute(std::span<expected_node> exp, string_t input) noexcept
      {
        parse_helper p;
        auto ast = p(input);
        tree_checker{ exp }(ast);
      }

      void check_error(tnac::string_t input, tnac::string_t errMsg) noexcept
      {
        parse_helper p;
        error_checker checker{ p, errMsg };
        p.parser(input);
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

  TEST(parser, t_struct_simple_unary)
  {
    using detail::expected_node;
    using enum detail::node_kind;
    constexpr auto input = "-42"sv;

    /*
          -'-'
         |
        42
    */

    std::array exp{
      expected_node{ "42", Literal, Unary },
      expected_node{  "-", Unary,   Scope }
    };

    detail::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_complex_unary)
  {
    using detail::expected_node;
    using enum detail::node_kind;
    constexpr auto input = "-(42 + 69)"sv;

    /*
                -'-'
               |
             -()
            |
          -'+'-
         |     |
        42     69
    */

    std::array exp{
      expected_node{ "42", Literal, Binary },
      expected_node{ "69", Literal, Binary },
      expected_node{  "+", Binary,  Paren },
      expected_node{   "", Paren,   Unary },
      expected_node{  "-", Unary,   Scope },
    };

    detail::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_simple_binary)
  {
    using detail::expected_node;
    using enum detail::node_kind;
    constexpr auto input = "42 + -2"sv;

    /*
          -'+'-
         |     |
        42    '-'
               |
               2
    */

    std::array exp{
      expected_node{ "42", Literal, Binary },
      expected_node{  "2", Literal, Unary },
      expected_node{  "-", Unary,   Binary },
      expected_node{  "+", Binary,  Scope }
    };

    detail::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_seq_binary)
  {
    using detail::expected_node;
    using enum detail::node_kind;
    constexpr auto input = "128 - 127-1"sv;

    /*
             --'-'-
            |      |
          -'-'-    1
         |     |
        128   127
    */

    std::array exp{
      expected_node{ "128", Literal, Binary },
      expected_node{ "127", Literal, Binary },
      expected_node{   "-", Binary,  Binary },
      expected_node{   "1", Literal, Binary },
      expected_node{   "-", Binary,  Scope },
    };

    detail::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_complex_binary)
  {
    using detail::expected_node;
    using enum detail::node_kind;
    constexpr auto input = "42 + -2 + 69 * 5 + 1"sv;

    /*
                       -------'+'--
                      |            |
              -------'+'--         1
             |            |
           -'+'-        -'*'- 
          |     |      |     |
         42    '-'    69     5
                |
                2
    */

    std::array exp{
      expected_node{ "42", Literal, Binary },
      expected_node{  "2", Literal, Unary },
      expected_node{  "-", Unary,   Binary },
      expected_node{  "+", Binary,  Binary },
      expected_node{ "69", Literal, Binary },
      expected_node{  "5", Literal, Binary },
      expected_node{  "*", Binary,  Binary },
      expected_node{  "+", Binary,  Binary },
      expected_node{  "1", Literal, Binary },
      expected_node{  "+", Binary,  Scope }
    };

    detail::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_op_precedence)
  {
    using detail::expected_node;
    using enum detail::node_kind;
    constexpr auto input = "42 * ( 1 + 2) + 2 * 69"sv;

    /*
              -----'+'---
             |           |
          --'*'--      -'*'-
         |       |    |     |
        42     -()    2     69
              |
            -'+'-
           |     |
           1     2
    */

    std::array exp{
      expected_node{ "42", Literal, Binary },
      expected_node{  "1", Literal, Binary },
      expected_node{  "2", Literal, Binary },
      expected_node{  "+", Binary,  Paren },
      expected_node{   "", Paren,   Binary },
      expected_node{  "*", Binary,  Binary },
      expected_node{  "2", Literal, Binary },
      expected_node{ "69", Literal, Binary },
      expected_node{  "*", Binary,  Binary },
      expected_node{  "+", Binary,  Scope },
    };

    detail::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_simple_decl)
  {
    using detail::expected_node;
    using enum detail::node_kind;
    constexpr auto input = "a = 42 + 1"sv;

    /*
                -decl-expr
               |
             -'a'
            |
          -'+'-
         |     |
        42     1
    */

    std::array exp{
      expected_node{ "42", Literal, Binary },
      expected_node{  "1", Literal, Binary },
      expected_node{  "+", Binary,  VarDecl },
      expected_node{  "a", VarDecl, Decl },
      expected_node{   "", Decl,    Scope },
    };

    detail::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_complex_decl)
  {
    using detail::expected_node;
    using enum detail::node_kind;
    constexpr auto input = "a = b = c = 42 + 1"sv;

    /*
                                  -decl-expr
                                 |
                               -'a'
                              |
                         -decl-expr
                        |
                      -'b'
                     |
                -decl-expr
               |
             -'c'
            |
          -'+'-
         |     |
        42     1
    */

    std::array exp{
      expected_node{ "42", Literal, Binary },
      expected_node{  "1", Literal, Binary },
      expected_node{  "+", Binary,  VarDecl },
      expected_node{  "c", VarDecl, Decl },
      expected_node{   "", Decl,    VarDecl },
      expected_node{  "b", VarDecl, Decl },
      expected_node{   "", Decl,    VarDecl },
      expected_node{  "a", VarDecl, Decl },
      expected_node{   "", Decl,    Scope },
    };

    detail::check_tree_structute(exp, input);
  }

  TEST(parser, t_errors)
  {
    using detail::check_error;
    check_error("2 + "sv, "Expected expression"sv);
    check_error("--2"sv, "Expected expression"sv);
    check_error("1 + 1 2"sv, "Expected ':' or EOL"sv);
    check_error("a"sv, "Expected initialisation"sv);
    check_error("a + 2"sv, "Expected initialisation"sv);
    check_error("1 + 1 = 2"sv, "Expected a single identifier"sv);
    check_error("1 + a"sv, "Undefined identifier"sv);
    check_error("2*(1 + 2"sv, "Expected ')'"sv);
  }
}