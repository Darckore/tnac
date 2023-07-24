#include "test_cases/test_common.hpp"
#include "ast/ast_visitor.hpp"

namespace tnac_tests
{
  namespace
  {
    using enum tree::node_kind;
  }

  TEST(parser, t_literals)
  {
    constexpr std::array inputArr{
      "0"sv, "42"sv, "042"sv, "0b1101"sv, "0xfF2"sv, "42.69"sv
    };

    tree_checker::check_simple_exprs<node_kind::Literal>(inputArr);
  }

  TEST(parser, t_unaries)
  {
    constexpr std::array inputArr{
      "+0"sv, "-42"sv, "+042"sv, "+0b1101"sv, "-0xfF2"sv, "-42.69"sv, "~42"sv
    };

    tree_checker::check_simple_exprs<node_kind::Unary>(inputArr);
  }

  TEST(parser, t_binaries)
  {
    constexpr std::array inputArr{
      "+0 - 1"sv, "-42 + 0xff"sv, "042 * 2"sv, "0b1101 + -0.5"sv, "0*0xfF2"sv, "-42.69 / 0.0 / 2"sv
    };

    tree_checker::check_simple_exprs<node_kind::Binary>(inputArr);
  }

  TEST(parser, t_struct_simple_unary)
  {
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

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_complex_unary)
  {
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

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_abs)
  {
    constexpr auto input = "|42 + 69|"sv;

    /*
             -||
            |
          -'+'-
         |     |
        42     69
    */

    std::array exp{
      expected_node{ "42", Literal, Binary },
      expected_node{ "69", Literal, Binary },
      expected_node{ "+",  Binary,  Abs },
      expected_node{ "",   Abs,     Scope },
    };

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_simple_binary)
  {
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

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_result_unary)
  {
    constexpr auto input = "-_result"sv;

    /*
             -'-'
            |
        _result
    */

    std::array exp{
      expected_node{ "_result", Result, Unary },
      expected_node{  "-", Unary,  Scope }
    };

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_result_bin)
  {
    constexpr auto input = "_result + 1"sv;

    /*
             -'+'---
            |       |
        _result     1
    */

    std::array exp{
      expected_node{ "_result", Result, Binary },
      expected_node{  "1", Literal, Binary },
      expected_node{  "+", Binary,  Scope }
    };

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_seq_binary)
  {
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

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_pow_root)
  {
    constexpr auto input = "10 ** 2 * 4 // 5"sv;

    /*
               -----'*'-----
              |             |
           --'**'--      --'//'--
          |        |    |        |
         10        2    4        5
    */

    std::array exp{
      expected_node{ "10", Literal, Binary },
      expected_node{ "2",  Literal, Binary },
      expected_node{ "**", Binary,  Binary },
      expected_node{ "4",  Literal, Binary },
      expected_node{ "5",  Literal, Binary },
      expected_node{ "//", Binary,  Binary },
      expected_node{ "*",  Binary,  Scope },
    };

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_complex_binary)
  {
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

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_call)
  {
    constexpr auto input = "f(a) ; : f(1+2)"sv;

    /*
             --call-expr--
            |             |
           'f'         --'+'--
                      |       |
                      1       2
    */

    std::array exp{
      expected_node{ "f", Identifier, Call },
      expected_node{ "1", Literal,    Binary },
      expected_node{ "2", Literal,    Binary },
      expected_node{ "+", Binary,     Call },
      expected_node{ "",  Call,       Scope },
    };

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_typed)
  {
    constexpr auto input = "_cplx(1,2)"sv;

    /*
         --'_cplx'--
        |           |
        1           2
    */

    std::array exp{
      expected_node{     "1", Literal, Typed },
      expected_node{     "2", Literal, Typed },
      expected_node{ "_cplx", Typed,   Scope },
    };

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_complex_typed)
  {
    constexpr auto input = "_cplx(1 + 2, 2 * 2 + 3)"sv;

    /*
               --'_cplx'-----
              |              |
           --'+'--        --'+'--
          |       |      |       |
          1       2   --'*'--    3
                     |       |
                     2       2
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
      expected_node{ "_cplx", Typed,   Scope },
    };

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_op_precedence)
  {
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

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_bitwise_precedence)
  {
    constexpr auto input = "1 | 2 ^ 3 & 4 + 5 & 6 | 7 ^ 8"sv;

    /*
            --'|'-----------
           |                |
        --'|'--          --'^'--
       |       |        |       |
      '1'   --'^'--    '7'     '8'
           |       |
          '2'   --'&'--
               |       |
            --'&'--   '6'
           |       |
          '3'   --'+'--
               |       |
              '4'     '5'
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
      expected_node{  "|", Binary,  Scope  },
    };

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_simple_decl)
  {
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

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_complex_decl)
  {
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

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_func_decl_empty)
  {
    constexpr auto input = "f() ;"sv;

    /*
             - decl-expr
            |
          -'f'-
         |     |
        { }   { }
    */

    std::array exp{
      expected_node{   "", Scope,    FuncDecl },
      expected_node{  "f", FuncDecl, Decl },
      expected_node{   "", Decl,     Scope },
    };

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_func_decl_params)
  {
    constexpr auto input = "f(p1, p2, p3) ;"sv;

    /*
           - decl-expr
          |
        -'f'----------
       |              |
       |- 'p1'       { }
       |- 'p2'
       |- 'p3'
    */

    std::array exp{
      expected_node{ "p1", ParamDecl, FuncDecl },
      expected_node{ "p2", ParamDecl, FuncDecl },
      expected_node{ "p3", ParamDecl, FuncDecl },
      expected_node{   "", Scope,     FuncDecl },
      expected_node{  "f", FuncDecl,  Decl },
      expected_node{   "", Decl,      Scope },
    };

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_struct_func_decl_body)
  {
    constexpr auto input = "f(p) p+1;"sv;

    /*
            - decl-expr
           |
         -'f'-----
        |         |
        |- 'p'    |------
                         |
                      --'+'--
                     |       |
                    'p'     '1'
    */

    std::array exp{
      expected_node{  "p", ParamDecl,  FuncDecl },
      expected_node{  "p", Identifier, Binary },
      expected_node{  "1", Literal,    Binary },
      expected_node{  "+", Binary,     Scope },
      expected_node{   "", Scope,      FuncDecl },
      expected_node{  "f", FuncDecl,   Decl },
      expected_node{   "", Decl,       Scope },
    };

    tree_checker::check_tree_structute(exp, input);
  }

  TEST(parser, t_errors)
  {
    tree_checker::check_error("2 + "sv, "Expected expression"sv);
    tree_checker::check_error("--2"sv, "Expected expression"sv);
    tree_checker::check_error("1 + 1 2"sv, "Expected ':' or EOL"sv);
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
    auto core = get_tnac();
    auto&& parser = core.get_parser();

    auto ast = parser("#command p1 p2"sv);
    ASSERT_NE(ast, nullptr);
    EXPECT_TRUE(ast->is(Scope));

    ast = parser("#command p1 p2 : 2 + 2"sv);
    ASSERT_NE(ast, nullptr);
    EXPECT_TRUE(ast->is(Binary));

    ast = parser("-2 #cmd : 3 + 3"sv);
    ASSERT_NE(ast, nullptr);
    EXPECT_TRUE(ast->is(Binary));
  }

  TEST(parser, t_cmd_handle)
  {
    auto core = get_tnac();
    auto&& parser = core.get_parser();
    core.on_command([](tnac::ast::command) noexcept {});

    auto ast = parser("#command p1 p2"sv);
    ASSERT_NE(ast, nullptr);
    EXPECT_TRUE(ast->is(Scope));

    ast = parser("#command p1 p2 : 2 + 2"sv);
    ASSERT_NE(ast, nullptr);
    EXPECT_TRUE(ast->is(Binary));

    ast = parser("-2 #cmd : 3 + 3"sv);
    ASSERT_NE(ast, nullptr);
    EXPECT_TRUE(ast->is(Binary));
  }
}