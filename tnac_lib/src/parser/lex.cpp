#include "parser/lex.hpp"

namespace tnac
{
  namespace detail
  {
    namespace
    {
      template <std::size_t Size>
      constexpr auto is_in_range(char_t c, const std::array<char_t, Size>& arr) noexcept
      {
        auto beg = arr.begin();
        auto end = arr.end();
        return std::find(beg, end, c) != end;
      }

      constexpr auto is_digit(char_t c) noexcept
      {
        constexpr std::array digits{
          '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
        };

        return is_in_range(c, digits);
      }
      constexpr auto is_operator(char_t c) noexcept
      {
        constexpr std::array ops{
          '+', '-', '*', '/'
        };

        return is_in_range(c, ops);
      }
      constexpr auto is_blank(char_t c) noexcept
      {
        constexpr std::array blanks{
          ' ', '\n', '\t', '\f', '\v', '\r'
        };

        return is_in_range(c, blanks);
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
      constexpr auto is_nonzero_digit(char_t c) noexcept
      {
        return c != '0' && is_digit(c);
      }
    }
  }


  // Public members

  void lex::feed(string_t buf) noexcept
  {
    m_buf = buf;
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

    if (!digit_seq())
      return consume(Error);

    if (detail::is_dot(peek_char()))
    {
      advance();
      if (digit_seq() && detail::is_separator(peek_char()))
        return consume(Float);

      return consume(Error);
    }

    return consume(IntDec);
  }

  bool lex::digit_seq() noexcept
  {
    bool ok = false;
    while (good())
    {
      const auto next = peek_char();
      if (detail::is_digit(next))
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
    ++m_to;
  }

  bool lex::good() const noexcept
  {
    return m_to != m_buf.end();
  }

}