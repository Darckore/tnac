#include "test_cases/test_common.hpp"
#include "parser/ast/ast_visitor.hpp"
#include "common/feedback.hpp"

namespace tnac::tests
{
  namespace
  {
    using enum ast::node_kind;

    class tree_checker : public ast::const_bottom_up_visitor<tree_checker>
    {
    public:
      struct expected_node
      {
        string_t data{};
        ast::node_kind kind{};
        ast::node_kind parent{};
        bool nullParent{};
      };

      using test_data = std::span<expected_node>;
      using data_iter = test_data::iterator;

      static void check_tree_structure(std::span<expected_node> exp, string_t input) noexcept
      {
        feedback fb;
        fb.on_parse_error(unexpected_err);
        auto core = get_tnac(fb);
        auto tree = core.parse(input);
        tree_checker{ exp }(tree);
      }

      template <ast::node_kind kind, std::size_t N>
      static void check_simple_exprs(const std::array<string_t, N>& inputs)
      {
        feedback fb;
        auto core = get_tnac(fb);
        for (auto input : inputs)
        {
          auto tree = core.parse(input);
          EXPECT_NE(tree, nullptr) << "Null AST for input: " << input;

          if (tree)
          {
            const auto nodeKind = tree->what();
            EXPECT_EQ(nodeKind, kind) << "Bad kind for input: " << input;
            if (nodeKind == kind)
            {
              auto&& tok = static_cast<ast::expr&>(*tree).pos();
              EXPECT_TRUE(input.starts_with(tok.value()));
            }
          }
        }
      }

      static void check_error(string_t input, string_t errMsg) noexcept
      {
        feedback fb;
        fb.on_parse_error(on_error);
        auto core = get_tnac(fb);
        expectedErr = errMsg;
        stop = false;

        errCount = {};
        core.parse(input);

        expectedErr = {};
        stop = false;
        ASSERT_TRUE(errCount > 0u) << "Got no errors for input: " << input;
      }

    private:
      inline static string_t expectedErr{};
      inline static bool stop{};
      inline static unsigned errCount{};

      static void on_error(const ast::error_expr& err) noexcept
      {
        ++errCount;
        if (stop) return;

        if (err.message() == expectedErr)
        {
          stop = true;
          return;
        }

        if (err.message() != expectedErr)
        {
          ASSERT_EQ(err.message(), expectedErr);
        }
      }

      static void unexpected_err(const ast::error_expr& err) noexcept
      {
        FAIL() << "Unexpected error: '" << err.message() << "'";
      }

    public:
      CLASS_SPECIALS_NONE(tree_checker);

      tree_checker(test_data expected) noexcept :
        m_data{ expected },
        m_iter{ expected.begin() }
      {}

    public:
      void visit(const ast::scope& scope) noexcept
      {
        check_node(scope, {});
      }

      void visit(const ast::assign_expr& expr) noexcept
      {
        check_node(expr, expr.op().value());
      }

      void visit(const ast::decl_expr& expr) noexcept
      {
        check_node(expr, {});
      }

      void visit(const ast::var_decl& decl) noexcept
      {
        check_node(decl, decl.name());
      }

      void visit(const ast::param_decl& decl) noexcept
      {
        check_node(decl, decl.name());
      }

      void visit(const ast::func_decl& decl) noexcept
      {
        check_node(decl, decl.name());
      }

      void visit(const ast::cond_short& cond) noexcept
      {
        check_node(cond, {});
      }

      void visit(const ast::cond_expr& cond) noexcept
      {
        check_node(cond, {});
      }

      void visit(const ast::pattern& pat) noexcept
      {
        check_node(pat, {});
      }

      void visit(const ast::matcher& mat) noexcept
      {
        auto opStr = ""sv;
        if (mat.is_unary())
          opStr = mat.pos().value();
        else if (mat.has_implicit_op())
          opStr = "=="sv;
        else if(!mat.is_default())
          opStr = mat.pos().value();

        check_node(mat, opStr);
      }

      void visit(const ast::binary_expr& expr) noexcept
      {
        check_node(expr, expr.op().value());
      }

      void visit(const ast::unary_expr& expr) noexcept
      {
        check_node(expr, expr.op().value());
      }

      void visit(const ast::paren_expr& expr) noexcept
      {
        check_node(expr, {});
      }

      void visit(const ast::abs_expr& expr) noexcept
      {
        check_node(expr, {});
      }

      void visit(const ast::typed_expr& expr) noexcept
      {
        check_node(expr, expr.type_name().value());
      }

      void visit(const ast::call_expr& expr) noexcept
      {
        check_node(expr, {});
      }

      void visit(const ast::lit_expr& expr) noexcept
      {
        check_node(expr, expr.pos().value());
      }

      void visit(const ast::id_expr& expr) noexcept
      {
        check_node(expr, expr.name());
      }

      void visit(const ast::ret_expr& expr) noexcept
      {
        check_node(expr, {});
      }

      void visit(const ast::result_expr& expr) noexcept
      {
        check_node(expr, expr.pos().value());
      }

      void visit(const ast::error_expr& expr) noexcept
      {
        check_node(expr, expr.message());
      }

    private:
      void check_node(const ast::node& node, string_t nodeStr) noexcept
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

    using expected_node = tree_checker::expected_node;
  }
}

namespace tnac::tests
{
  TEST(parser, t_literals)
  {
    constexpr std::array inputArr{
      "0"sv, "42"sv, "042"sv, "0b1101"sv, "0xfF2"sv, "42.69"sv
    };

    tree_checker::check_simple_exprs<ast::node_kind::Literal>(inputArr);
  }

  TEST(parser, t_unaries)
  {
    constexpr std::array inputArr{
      "+0"sv, "-42"sv, "+042"sv, "+0b1101"sv, "-0xfF2"sv, "-42.69"sv, "~42"sv
    };

    tree_checker::check_simple_exprs<ast::node_kind::Unary>(inputArr);
  }

  TEST(parser, t_binaries)
  {
    constexpr std::array inputArr{
      "+0 - 1"sv, "-42 + 0xff"sv, "042 * 2"sv, "0b1101 + -0.5"sv, "0*0xfF2"sv, "-42.69 / 0.0 / 2"sv
    };

    tree_checker::check_simple_exprs<ast::node_kind::Binary>(inputArr);
  }

  TEST(parser, t_struct_simple_unary)
  {
    constexpr auto input = "-42"sv;

    /*
    * -
    *   42
    */

    std::array exp{
      expected_node{ "42", Literal, Unary },
      expected_node{  "-", Unary,   Module }
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_complex_unary)
  {
    constexpr auto input = "-(42 + 69)"sv;

    /*
    * -
    *   ()
    *     +
    *       42
    *       69
    */

    std::array exp{
      expected_node{ "42", Literal, Binary },
      expected_node{ "69", Literal, Binary },
      expected_node{  "+", Binary,  Paren },
      expected_node{   {}, Paren,   Unary},
      expected_node{  "-", Unary,   Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_abs)
  {
    constexpr auto input = "|42 + 69|"sv;

    /*
    * ||
    *   +
    *     42
    *     69
    */

    std::array exp{
      expected_node{ "42", Literal, Binary },
      expected_node{ "69", Literal, Binary },
      expected_node{ "+",  Binary,  Abs },
      expected_node{ {},   Abs,     Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_simple_binary)
  {
    constexpr auto input = "42 + -2"sv;

    /*
    * +
    *   42
    *   -
    *     2
    */

    std::array exp{
      expected_node{ "42", Literal, Binary },
      expected_node{  "2", Literal, Unary },
      expected_node{  "-", Unary,   Binary },
      expected_node{  "+", Binary,  Module }
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_result_unary)
  {
    constexpr auto input = "-_result"sv;

    /*
    * -
    *   _result
    */

    std::array exp{
      expected_node{ "_result", Result, Unary },
      expected_node{  "-", Unary,  Module }
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_result_bin)
  {
    constexpr auto input = "_result + 1"sv;

    /*
    * +
    *   _result
    *   1
    */

    std::array exp{
      expected_node{ "_result", Result, Binary },
      expected_node{  "1", Literal, Binary },
      expected_node{  "+", Binary,  Module }
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_seq_binary) // :3
  {
    constexpr auto input = "128 - 127-1"sv;

    /*
    * -
    *   -
    *     128
    *     127
    *   1
    */

    std::array exp{
      expected_node{ "128", Literal, Binary },
      expected_node{ "127", Literal, Binary },
      expected_node{   "-", Binary,  Binary },
      expected_node{   "1", Literal, Binary },
      expected_node{   "-", Binary,  Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_pow_root)
  {
    constexpr auto input = "10 ** 2 * 4 // 5"sv;

    /*
    * *
    *   **
    *     10
    *     2
    *   //
    *     4
    *     5
    */

    std::array exp{
      expected_node{ "10", Literal, Binary },
      expected_node{ "2",  Literal, Binary },
      expected_node{ "**", Binary,  Binary },
      expected_node{ "4",  Literal, Binary },
      expected_node{ "5",  Literal, Binary },
      expected_node{ "//", Binary,  Binary },
      expected_node{ "*",  Binary,  Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_complex_binary)
  {
    constexpr auto input = "42 + -2 + 69 * 5 + 1"sv;

    /*
    * +
    *   +
    *     +
    *       42
    *       -
    *         2
    *     *
    *       69
    *       5
    *   1
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
      expected_node{  "+", Binary,  Module }
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_call)
  {
    constexpr auto input = "f(a) ; : f(1+2)"sv;

    /*
    * call
    *     f
    *     +
    *       1
    *       2
    */

    std::array exp{
      expected_node{ "f", Identifier, Call },
      expected_node{ "1", Literal,    Binary },
      expected_node{ "2", Literal,    Binary },
      expected_node{ "+", Binary,     Call },
      expected_node{ {},  Call,       Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_typed)
  {
    constexpr auto input = "_cplx(1,2)"sv;

    /*
    * _cplx
    *     1
    *     2
    */

    std::array exp{
      expected_node{     "1", Literal, Typed },
      expected_node{     "2", Literal, Typed },
      expected_node{ "_cplx", Typed,   Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_complex_typed)
  {
    constexpr auto input = "_cplx(1 + 2, 2 * 2 + 3)"sv;

    /*
    * _cplx
    *     +
    *       1
    *       2
    *     +
    *       *
    *         2
    *         2
    *       3
    */

    std::array exp{
      expected_node{     "1", Literal, Binary },
      expected_node{     "2", Literal, Binary },
      expected_node{     "+", Binary,  Typed },
      expected_node{     "2", Literal, Binary },
      expected_node{     "2", Literal, Binary },
      expected_node{     "*", Binary,  Binary },
      expected_node{     "3", Literal, Binary },
      expected_node{     "+", Binary,  Typed },
      expected_node{ "_cplx", Typed,   Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_op_precedence)
  {
    constexpr auto input = "42 * ( 1 + 2) + 2 * 69"sv;

    /*
    * +
    *   *
    *     42
    *     ()
    *       +
    *         1
    *         2
    *   *
    *     2
    *     69
    */

    std::array exp{
      expected_node{ "42", Literal, Binary },
      expected_node{  "1", Literal, Binary },
      expected_node{  "2", Literal, Binary },
      expected_node{  "+", Binary,  Paren },
      expected_node{   {}, Paren,   Binary },
      expected_node{  "*", Binary,  Binary },
      expected_node{  "2", Literal, Binary },
      expected_node{ "69", Literal, Binary },
      expected_node{  "*", Binary,  Binary },
      expected_node{  "+", Binary,  Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_bitwise_precedence)
  {
    constexpr auto input = "1 | 2 ^ 3 & 4 + 5 & 6 | 7 ^ 8"sv;
    /*
    * |
    *   |
    *     1
    *     ^
    *       2
    *       &
    *         &
    *           3
    *           +
    *             4
    *             5
    *       6
    *   ^
    *     7
    *     8
    */

    std::array exp{
      expected_node{  "1", Literal, Binary },
      expected_node{  "2", Literal, Binary },
      expected_node{  "3", Literal, Binary },
      expected_node{  "4", Literal, Binary },
      expected_node{  "5", Literal, Binary },
      expected_node{  "+", Binary,  Binary },
      expected_node{  "&", Binary,  Binary },
      expected_node{  "6", Literal, Binary },
      expected_node{  "&", Binary,  Binary },
      expected_node{  "^", Binary,  Binary },
      expected_node{  "|", Binary,  Binary },
      expected_node{  "7", Literal, Binary },
      expected_node{  "8", Literal, Binary },
      expected_node{  "^", Binary,  Binary },
      expected_node{  "|", Binary,  Module  },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_simple_decl)
  {
    constexpr auto input = "a = 42 + 1"sv;

    /*
    * decl
    *   a
    *   +
    *     42
    *     1
    */

    std::array exp{
      expected_node{ "42", Literal, Binary },
      expected_node{  "1", Literal, Binary },
      expected_node{  "+", Binary,  VarDecl },
      expected_node{  "a", VarDecl, Decl },
      expected_node{   {}, Decl,    Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_complex_decl)
  {
    constexpr auto input = "a = b = c = 42 + 1"sv;

    /*
    * decl
    *   a
    *   decl
    *     b
    *     decl
    *       c
    *       +
    *         42
    *         1
    */

    std::array exp{
      expected_node{ "42", Literal, Binary },
      expected_node{  "1", Literal, Binary },
      expected_node{  "+", Binary,  VarDecl },
      expected_node{  "c", VarDecl, Decl },
      expected_node{   {}, Decl,    VarDecl },
      expected_node{  "b", VarDecl, Decl },
      expected_node{   {}, Decl,    VarDecl },
      expected_node{  "a", VarDecl, Decl },
      expected_node{   {}, Decl,    Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_func_decl_empty)
  {
    constexpr auto input = "f() ;"sv;

    /*
    * decl
    *   f
    *     ()
    *     {}
    */

    std::array exp{
      expected_node{   {}, Scope,    FuncDecl },
      expected_node{  "f", FuncDecl, Decl },
      expected_node{   {}, Decl,     Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_func_decl_params)
  {
    constexpr auto input = "f(p1, p2, p3) ;"sv;

    /*
    * decl
    *   f
    *   (p1, p2, p3)
    *   {}
    */

    std::array exp{
      expected_node{ "p1", ParamDecl,  FuncDecl },
      expected_node{ "p2", ParamDecl,  FuncDecl },
      expected_node{ "p3", ParamDecl,  FuncDecl },
      expected_node{   {}, Scope,      FuncDecl },
      expected_node{  "f", FuncDecl,   Decl },
      expected_node{   {}, Decl,       Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_func_decl_body)
  {
    constexpr auto input = "f(p) p+1;"sv;

    /*
    * decl
    *   f
    *     (p)
    *     {
    *       +
    *         p
    *         1
    *     }
    */

    std::array exp{
      expected_node{  "p", ParamDecl,  FuncDecl },
      expected_node{  "p", Identifier, Binary },
      expected_node{  "1", Literal,    Binary },
      expected_node{  "+", Binary,     Scope },
      expected_node{   {}, Scope,      FuncDecl },
      expected_node{  "f", FuncDecl,   Decl },
      expected_node{   {}, Decl,       Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_ret)
  {
    constexpr auto input = "f() _ret fn(p) p+1;;"sv;

    /*
    * decl
    *   f
    *   ()
    *   {
    *     _ret
    *       decl
    *         _fn
    *         (p)
    *         {
    *           +
    *             p
    *             1
    *         }
    *   }
    */

    std::array exp{
      expected_node{   "p", ParamDecl,  FuncDecl },
      expected_node{   "p", Identifier, Binary },
      expected_node{   "1", Literal,    Binary },
      expected_node{   "+", Binary,     Scope },
      expected_node{    {}, Scope,      FuncDecl },
      expected_node{  "fn", FuncDecl,   Decl },
      expected_node{    {}, Decl,       Ret },
      expected_node{    {}, Ret,        Scope},
      expected_node{    {}, Scope,      FuncDecl },
      expected_node{   "f", FuncDecl,   Decl },
      expected_node{    {}, Decl,       Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_cond_short)
  {
    constexpr auto input = "a = 42: { a > 0 }->{ a + 1, a - 2 }"sv;

    /*
    * cond-short
    *   >
    *     a
    *     0
    *   +
    *     a
    *     1
    *   -
    *     a
    *     2
    */

    std::array exp{
      expected_node{   "a", Identifier, Binary },
      expected_node{   "0", Literal,    Binary },
      expected_node{   ">", Binary,     CondShort },
      expected_node{   "a", Identifier, Binary },
      expected_node{   "1", Literal,    Binary },
      expected_node{   "+", Binary,     CondShort },
      expected_node{   "a", Identifier, Binary },
      expected_node{   "2", Literal,    Binary },
      expected_node{   "-", Binary,     CondShort },
      expected_node{    {}, CondShort,  Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_cond_short_true)
  {
    constexpr auto input = "a = 42: { a > 0 }->{ a + 1 }"sv;

    /*
    * cond-short
    *   >
    *     a
    *     0
    *   +
    *     a
    *     1
    *   {}
    */

    std::array exp{
      expected_node{   "a", Identifier, Binary },
      expected_node{   "0", Literal,    Binary },
      expected_node{   ">", Binary,     CondShort },
      expected_node{   "a", Identifier, Binary },
      expected_node{   "1", Literal,    Binary },
      expected_node{   "+", Binary,     CondShort },
      expected_node{    {}, CondShort,  Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_cond_short_false)
  {
    constexpr auto input = "a = 42: { a > 0 }->{ , a - 2 }"sv;

    /*
    * cond-short
    *   >
    *     a
    *     0
    *   {}
    *   -
    *     a
    *     2
    */

    std::array exp{
      expected_node{   "a", Identifier, Binary },
      expected_node{   "0", Literal,    Binary },
      expected_node{   ">", Binary,     CondShort },
      expected_node{   "a", Identifier, Binary },
      expected_node{   "2", Literal,    Binary },
      expected_node{   "-", Binary,     CondShort},
      expected_node{    {}, CondShort,  Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_struct_cond)
  {
    constexpr auto input = "a = 42: { a } { ! }->1; { 42 }->2; { <69 }->3; {}->4; ;"sv;

    /*
    * cond
    *   a
    *   {
    *     pattern
    *       matcher !
    *         {
    *           1
    *         }
    *     pattern
    *       matcher ==
    *         1
    *         {
    *           2
    *         }
    *     pattern
    *       matcher <
    *         1
    *         {
    *           3
    *         }
    *     pattern
    *       matcher {}
    *         {
    *           4
    *         }
    *   }
    */

    std::array exp{
      expected_node{ "a"sv,   Identifier, Cond },
      expected_node{ "!"sv,   Matcher,    Pattern},
      expected_node{ "1"sv,   Literal,    Scope },
      expected_node{    {},   Scope,      Pattern },
      expected_node{    {},   Pattern,    Scope },
      expected_node{ "42"sv,  Literal,    Matcher },
      expected_node{ "=="sv,  Matcher,    Pattern },
      expected_node{  "2"sv,  Literal,    Scope },
      expected_node{     {},  Scope,      Pattern },
      expected_node{     {},  Pattern,    Scope },
      expected_node{ "69"sv,  Literal,    Matcher },
      expected_node{  "<"sv,  Matcher,    Pattern },
      expected_node{  "3"sv,  Literal,    Scope },
      expected_node{     {},  Scope,      Pattern },
      expected_node{     {},  Pattern,    Scope },

      expected_node{     {},  Matcher,    Pattern},
      expected_node{  "4"sv,  Literal,    Scope },
      expected_node{     {},  Scope,      Pattern },
      expected_node{     {},  Pattern,    Scope },

      expected_node{     {},  Scope,      Cond },
      expected_node{     {},  Cond,       Module },
    };

    tree_checker::check_tree_structure(exp, input);
  }

  TEST(parser, t_errors)
  {
    tree_checker::check_error("2 + "sv, "Expected expression"sv);
    tree_checker::check_error("1 + 1 2"sv, "Expected ':' or EOL"sv);
    tree_checker::check_error("f() 1 + 1 2;"sv, "Expected ':' or EOL"sv);
    tree_checker::check_error("a"sv, "Expected initialisation"sv);
    tree_checker::check_error("a + 2"sv, "Expected initialisation"sv);
    tree_checker::check_error("1 + 1 = 2"sv, "Expected a single identifier"sv);
    tree_checker::check_error("1 + a"sv, "Undefined identifier"sv);
    tree_checker::check_error("2*(1 + 2"sv, "Expected ')'"sv);
    tree_checker::check_error("f() ; f = 10"sv, "Expected an assignable object"sv);
    tree_checker::check_error("f(a, a) ;"sv, "Function parameter redifinition"sv);
  }
  
  TEST(parser, t_cmd_skip)
  {
    feedback fb;
    auto core = get_tnac(fb);

    auto ast = core.parse("#command p1 p2"sv);
    ASSERT_NE(ast, nullptr);
    EXPECT_TRUE(ast->is(Root));

    ast = core.parse("#command p1 p2 : 2 + 2"sv);
    ASSERT_NE(ast, nullptr);
    EXPECT_TRUE(ast->is(Binary));

    ast = core.parse("-2 #cmd : 3 + 3"sv);
    ASSERT_NE(ast, nullptr);
    EXPECT_TRUE(ast->is(Binary));
  }

  TEST(parser, t_cmd_handle)
  {
    feedback fb;
    auto core = get_tnac(fb);
    fb.on_command([](ast::command) noexcept {});

    auto ast = core.parse("#command p1 p2"sv);
    ASSERT_NE(ast, nullptr);
    EXPECT_TRUE(ast->is(Root));

    ast = core.parse("#command p1 p2 : 2 + 2"sv);
    ASSERT_NE(ast, nullptr);
    EXPECT_TRUE(ast->is(Binary));

    ast = core.parse("-2 #cmd : 3 + 3"sv);
    ASSERT_NE(ast, nullptr);
    EXPECT_TRUE(ast->is(Binary));
  }
}