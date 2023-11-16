#include "test_cases/test_common.hpp"

namespace tnac::tests
{
  namespace
  {
    void all_same(string_t input, tok_kind kind)
    {
      lex l;
      l(input);

      for (;;)
      {
        auto tok = l.next();
        if (tok.is_eol())
          break;

        EXPECT_TRUE(tok.is(kind)) << "Failed token: " << tok.value();
      }
    }

    template <std::size_t N>
    void check_tokens(string_t input, const std::array<tok_kind, N>& tokArr) noexcept
    {
      lex l;
      l(input);

      for (auto tk : tokArr)
      {
        auto tok = l.next();
        EXPECT_TRUE(tok.is(tk)) << "Failed token: " << tok.value();
      }
    }
  }
}

namespace tnac::tests
{
  TEST(lexer, t_token_list)
  {
    constexpr auto input = ". = + - ~ * / && & ^ || | ** // ! ? == != < > <= >= -> : , ; ( ) { } [ ] _true _false 1 01 0b1 0x1 1.0 id #cmd"sv;

    using enum tok_kind;
    constexpr std::array testArr{
      Dot, Assign, Plus, Minus, Tilde, Asterisk, Slash,
      LogAnd, Amp, Hat, LogOr, Pipe, Pow, Root, Exclamation, Question,
      Eq, NotEq, Less, Greater, LessEq, GreaterEq, Arrow,
      ExprSep, Comma, Semicolon,
      ParenOpen, ParenClose, CurlyOpen, CurlyClose, BracketOpen, BracketClose,
      KwTrue, KwFalse,
      IntDec, IntOct, IntBin, IntHex, Float, Identifier, Command,
      Eol, Eol, Eol
    };

    check_tokens(input, testArr);
  }

  TEST(lexer, t_token_list_dense)
  {
    constexpr auto input = ".=+-~*/&&&^|||**//!?<><=>===!=->:(){}[],;0.1"sv;

    using enum tok_kind;
    constexpr std::array testArr{
      Dot, Assign, Plus, Minus, Tilde, Asterisk, Slash,
      LogAnd, Amp, Hat, LogOr, Pipe, Pow, Root, Exclamation, Question,
      Less, Greater, LessEq, GreaterEq, Eq, NotEq, Arrow, ExprSep,
      ParenOpen, ParenClose, CurlyOpen, CurlyClose, BracketOpen, BracketClose,
      Comma, Semicolon, Float
    };

    check_tokens(input, testArr);
  }

  TEST(lexer, t_nums_good)
  {
    constexpr auto binInts = "0b0 0b1 0b11111 0b000000 0b011101101"sv;
    constexpr auto octInts = "01 001 042 067 0227 0666 0256"sv;
    constexpr auto decInts = "0 1 42 69 228 666 256"sv;
    constexpr auto hexInts = "0x0 0x1 0x1234567890AbCDeF 0x69 0x228a 0x666 0xfF"sv;
    constexpr auto floats  = "0.0 0.00000 0.1 0023.3450 13456.0"sv;

    using enum tok_kind;
    all_same(binInts, IntBin);
    all_same(octInts, IntOct);
    all_same(decInts, IntDec);
    all_same(hexInts, IntHex);
    all_same(floats, Float);
  }

  TEST(lexer, t_nums_bad)
  {
    constexpr auto failures = "0. 08 0xabcdr 0b 0x 0b2 256a"sv;

    using enum tok_kind;
    all_same(failures, Error);
  }

  TEST(lexer, t_strings_good)
  {
    constexpr auto input = "'one' 'and two  ' 'and 3 & 4 @$' '' '            '"sv;

    using enum tok_kind;
    all_same(input, String);
  }

  TEST(lexer, t_strings_bad)
  {
    constexpr auto input = "'this string has no end"sv;

    using enum tok_kind;
    all_same(input, Error);
  }

  TEST(lexer, t_ids)
  {
    constexpr auto ids = "a var_001 x____111__v long_identifier_name_of_doom"sv;
    using enum tok_kind;
    all_same(ids, Identifier);
  }

  TEST(lexer, t_keywords)
  {
    constexpr auto input = 
      "_fn _ret _result _cplx _frac _int _flt _bool _true _false _i _pi _e _entry"sv;

    using enum tok_kind;
    constexpr std::array testArr{
      KwFunction, KwRet,
      KwResult, KwComplex, KwFraction, KwInt, KwFloat, KwBool,
      KwTrue, KwFalse,
      KwI, KwPi, KwE,
      KwEntry
    };

    check_tokens(input, testArr);
  }

  TEST(lexer, t_peek)
  {
    constexpr auto input = "42 69"sv;
    lex l;
    l(input);

    auto tok = l.peek();
    EXPECT_EQ(tok, l.peek());
    EXPECT_EQ(tok, l.peek());
    EXPECT_EQ(tok, l.next());
    EXPECT_NE(tok, l.peek());
    
    utils::unused(l.next());
    EXPECT_TRUE(l.next().is_eol());
  }
}