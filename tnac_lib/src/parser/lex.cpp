#include "parser/lex.hpp"

namespace tnac
{
  namespace detail
  {
    namespace
    {
      template <typename It>
      constexpr auto is_in_range(char_t c, It beg, It end) noexcept
      {
        return std::find(beg, end, c) != end;
      }

      constexpr auto to_lower(char_t c) noexcept
      {
        return utils::in_range(c, 'A', 'Z') ? char_t{ c + ('a' - 'A') } : c;
      }
      constexpr auto is_digit(char_t c, unsigned base = 10u) noexcept
      {
        constexpr auto maxBase = 16u;
        if (base > maxBase)
          return false;

        constexpr std::array digits{
          '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };

        auto beg = digits.begin();
        auto end = beg + base;
        return is_in_range(to_lower(c), beg, end);
      }
      constexpr auto is_operator(char_t c) noexcept
      {
        constexpr std::array ops{
          '+', '-', '*', '/'
        };

        return is_in_range(c, ops.begin(), ops.end());
      }
      constexpr auto is_blank(char_t c) noexcept
      {
        constexpr std::array blanks{
          ' ', '\n', '\t', '\f', '\v', '\r', '\0'
        };

        return is_in_range(c, blanks.begin(), blanks.end());
      }
      constexpr auto is_separator(char_t c) noexcept
      {
        return is_blank(c) ||
               is_operator(c);
      }

      constexpr auto is_dot(char_t c) noexcept
      {
        return c == '.';
      }
      constexpr auto is_zero_digit(char_t c) noexcept
      {
        return c == '0';
      }
      constexpr auto is_nonzero_digit(char_t c) noexcept
      {
        return !is_zero_digit(c) && is_digit(c);
      }

      constexpr auto is_bin_prefix(char_t c) noexcept
      {
        return to_lower(c) == 'b';
      }
      constexpr auto is_hex_prefix(char_t c) noexcept
      {
        return to_lower(c) == 'x';
      }
    }
  }


  // Public members

  void lex::feed(string_t buf) noexcept
  {
    m_buf = utils::ltrim(buf);
    m_from = m_buf.begin();
    m_to   = m_from;
  }

  token lex::next() noexcept
  {
    if (!good())
      return { .m_value{}, .m_kind{ token::Eol } };

    const auto next = peek_char();

    if (detail::is_digit(next))
    {
      return number();
    }

    if (detail::is_operator(next))
    {
      return op();
    }

    return consume(tok_kind::Error);
  }


  // Private members

  token lex::consume(tok_kind kind) noexcept
  {
    if (kind == tok_kind::Error)
      ffwd();

    token res{ .m_value{ m_from, m_to }, .m_kind{ kind } };

    while (good())
    {
      const auto c = peek_char();
      if (detail::is_blank(c))
      {
        advance();
        continue;
      }

      break;
    }

    m_from = m_to;
    return res;
  }

  void lex::ffwd() noexcept
  {
    while (good())
    {
      const auto next = peek_char();
      if (detail::is_separator(next))
        break;
        
      advance();
    }
  }

  token lex::number() noexcept
  {
    using enum tok_kind;

    auto leadingZero = detail::is_zero_digit(peek_char());
    if (leadingZero)
    {
      advance();
      const auto next = peek_char();

      if (detail::is_separator(next))
        return consume(IntDec); // literal 0 is decimal
      
      if (detail::is_bin_prefix(next))
        return bin_number();

      if (detail::is_hex_prefix(next))
        return hex_number();

      if (constexpr auto oct = 8u; detail::is_digit(next, oct) && digit_seq(oct))
      {
        if (detail::is_separator(peek_char()))
          return consume(IntOct);
      }
    }

    return decimal_number(leadingZero);
  }
  token lex::bin_number() noexcept
  {
    return hex_bin_impl(false);
  }
  token lex::hex_number() noexcept
  {
    return hex_bin_impl(true);
  }
  token lex::hex_bin_impl(bool isHex) noexcept
  {
    using enum tok_kind;
    advance();

    const auto base = isHex ? 16u : 2u;
    const auto kind = isHex ? IntHex : IntBin;

    if (!digit_seq(base))
      return consume(Error);

    return detail::is_separator(peek_char()) ? consume(kind) : consume(Error);
  }

  token lex::decimal_number(bool leadingZero) noexcept
  {
    using enum tok_kind;
    constexpr auto dec = 10u;
    auto result = Error;

    if (!digit_seq(dec) && !detail::is_dot(peek_char()))
      return consume(Error);

    if (const auto next = peek_char(); detail::is_separator(next))
    {
      result = leadingZero ? Error : IntDec;
    }
    else if (detail::is_dot(next))
    {
      advance();
      if (digit_seq(dec) && detail::is_separator(peek_char()))
        result = Float;
    }

    return consume(result);
  }

  bool lex::digit_seq(unsigned base) noexcept
  {
    bool ok = false;
    while (good())
    {
      const auto next = peek_char();
      if (detail::is_digit(next, base))
      {
        ok = true;
        advance();
        continue;
      }

      if (detail::is_dot(next) || detail::is_separator(next))
        break;

      ok = false;
      break;
    }

    return ok;
  }

  token lex::op() noexcept
  {
    using enum tok_kind;
    const auto next = peek_char();
    advance();
    
    auto resKind = Eol;
    switch (next)
    {
    case '+':
      resKind = Plus;
      break;

    case '-':
      resKind = Minus;
      break;

    case '*':
      resKind = Asterisk;
      break;

    case '/':
      resKind = Slash;
      break;

    default:
      resKind = Error;
      break;
    }

    return consume(resKind);
  }

  char_t lex::peek_char() const noexcept
  {
    return good() ? *m_to : 0;
  }

  void lex::advance() noexcept
  {
    if(good())
      ++m_to;
  }

  bool lex::good() const noexcept
  {
    return m_to != m_buf.end();
  }

}