#include "parser/lex.hpp"

namespace tnac_tests
{
  namespace detail
  {
    using tnac::string_t;
    using tnac::tok_kind;

    bool all_same(string_t input, tok_kind kind)
    {
      tnac::lex lex;
      lex.feed(input);

      bool ok = true;
      for (;;)
      {
        auto tok = lex.next();
        if (tok.is_eol())
          break;

        if (!tok.is(kind))
        {
          ok = false;
          EXPECT_TRUE(ok) << "Failed token: '" << tok.m_value << "' ";
        }
      }

      return ok;
    }
  }

  TEST(lexer, t_nums_good)
  {
    constexpr auto binInts = "0b0 0b1 0b11111 0b000000 0b011101101"sv;
    constexpr auto octInts = "01 001 042 067 0227 0666 0256"sv;
    constexpr auto decInts = "0 1 42 69 228 666 256"sv;
    constexpr auto hexInts = "0x0 0x1 0x1234567890AbCDeF 0x69 0x228a 0x666 0xfF"sv;
    constexpr auto floats  = "0.0 0.00000 0.1 0023.3450 13456.0"sv;

    using enum tnac::tok_kind;
    EXPECT_TRUE(detail::all_same(binInts, IntBin));
    EXPECT_TRUE(detail::all_same(octInts, IntOct));
    EXPECT_TRUE(detail::all_same(decInts, IntDec));
    EXPECT_TRUE(detail::all_same(hexInts, IntHex));
    EXPECT_TRUE(detail::all_same(floats, Float));
  }

  TEST(lexer, t_nums_bad)
  {
    constexpr auto failures = "0. .1 08 1.2.3 0xabcdr 0xab.c 0b111.1 0b 0x 0b2 256a"sv;

    using enum tnac::tok_kind;
    EXPECT_TRUE(detail::all_same(failures, Error));
  }
}