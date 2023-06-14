#include "parser/lex.hpp"

namespace tnac_tests
{
  namespace detail
  {
    namespace
    {
      using tnac::string_t;
      using tnac::tok_kind;

      void all_same(string_t input, tok_kind kind)
      {
        tnac::lex lex;
        lex(input);

        for (;;)
        {
          auto tok = lex.next();
          if (tok.is_eol())
            break;

          EXPECT_TRUE(tok.is(kind)) << "Failed token: " << tok.m_value;
        }
      }

      template <std::size_t N>
      void check_tokens(string_t input, const std::array<tok_kind, N>& tokArr) noexcept
      {
        tnac::lex lex;
        lex(input);

        for (auto tk : tokArr)
        {
          auto tok = lex.next();
          EXPECT_TRUE(tok.is(tk)) << "Failed token: " << tok.m_value;
        }
      }
    }
  }

  TEST(lexer, t_token_list)
  {
    constexpr auto input = "= + - ~ * / & ^ | : , ; ( ) 1 01 0b1 0x1 1.0 id #cmd"sv;

    using enum tnac::tok_kind;
    constexpr std::array testArr{
      Assign, Plus, Minus, Tilde, Asterisk, Slash, Amp, Hat, Pipe,
      ExprSep, Comma, Semicolon, ParenOpen, ParenClose,
      IntDec, IntOct, IntBin, IntHex, Float, Identifier, Command,
      Eol, Eol, Eol
    };

    using detail::check_tokens;
    check_tokens(input, testArr);
  }

  TEST(lexer, t_token_list_dense)
  {
    constexpr auto input = "=+-~*/&^|:(),;0.1"sv;

    using enum tnac::tok_kind;
    constexpr std::array testArr{
      Assign, Plus, Minus, Tilde, Asterisk, Slash, Amp, Hat, Pipe,
      ExprSep, ParenOpen, ParenClose, Comma, Semicolon, Float
    };

    using detail::check_tokens;
    check_tokens(input, testArr);
  }

  TEST(lexer, t_nums_good)
  {
    constexpr auto binInts = "0b0 0b1 0b11111 0b000000 0b011101101"sv;
    constexpr auto octInts = "01 001 042 067 0227 0666 0256"sv;
    constexpr auto decInts = "0 1 42 69 228 666 256"sv;
    constexpr auto hexInts = "0x0 0x1 0x1234567890AbCDeF 0x69 0x228a 0x666 0xfF"sv;
    constexpr auto floats  = "0.0 0.00000 0.1 0023.3450 13456.0"sv;

    using enum tnac::tok_kind;
    using detail::all_same;
    all_same(binInts, IntBin);
    all_same(octInts, IntOct);
    all_same(decInts, IntDec);
    all_same(hexInts, IntHex);
    all_same(floats, Float);
  }

  TEST(lexer, t_nums_bad)
  {
    constexpr auto failures = "0. .1 08 1.2.3 0xabcdr 0xab.c 0b111.1 0b 0x 0b2 256a"sv;

    using enum tnac::tok_kind;
    using detail::all_same;
    all_same(failures, Error);
  }

  TEST(lexer, t_strings_good)
  {
    constexpr auto input = "'one' 'and two  ' 'and 3 & 4 @$' '' '            '"sv;

    using enum tnac::tok_kind;
    using detail::all_same;
    all_same(input, String);
  }

  TEST(lexer, t_strings_bad)
  {
    constexpr auto input = "'this string has no end"sv;

    using enum tnac::tok_kind;
    using detail::all_same;
    all_same(input, Error);
  }

  TEST(lexer, t_ids)
  {
    constexpr auto ids = "a var_001 x____111__v long_identifier_name_of_doom"sv;
    using enum tnac::tok_kind;
    using detail::all_same;
    all_same(ids, Identifier);
  }

  TEST(lexer, t_keywords)
  {
    constexpr auto input = "_function _result _complex _fraction"sv;

    using enum tnac::tok_kind;
    constexpr std::array testArr{
      KwFunction,
      KwResult,
      KwComplex,
      KwFraction
    };

    using detail::check_tokens;
    check_tokens(input, testArr);
  }

  TEST(lexer, t_peek)
  {
    constexpr auto input = "42 69"sv;
    tnac::lex lex;
    lex(input);

    auto tok = lex.peek();
    EXPECT_EQ(tok, lex.peek());
    EXPECT_EQ(tok, lex.peek());
    EXPECT_EQ(tok, lex.next());
    EXPECT_NE(tok, lex.peek());
    
    utils::unused(lex.next());
    EXPECT_TRUE(lex.next().is_eol());
  }
}