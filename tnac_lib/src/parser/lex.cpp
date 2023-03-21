#include "parser/lex.hpp"

namespace tnac
{
  // Special members

  lex::lex(string_t buf) noexcept
  {
    feed(buf);
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

    if (is_digit(next))
    {
      return number();
    }

    if (is_operator(next))
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
      if (is_blank(c))
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
      if (is_separator(next))
        break;
        
      advance();
    }
  }

  token lex::number() noexcept
  {
    using enum tok_kind;

    auto resKind = Eol;
    auto hasDot = false;
    while (good())
    {
      const auto next = peek_char();

      if (next == '.')
      {
        if (!hasDot)
        {
          hasDot = true;
          advance();
          continue;
        }
        else
        {
          resKind = Error;
          break;
        }
      }

      if (is_separator(next))
        break;

      if (!is_digit(next))
      {
        resKind = Error;
        break;
      }

      advance();
    }

    if (resKind != Error)
    {
      if (hasDot)
        resKind = Float;
      else
        resKind = IntDec;
    }

    return consume(resKind);
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

  bool lex::is_digit(char_t c) const noexcept
  {
    return std::isdigit(c);
  }
  bool lex::is_operator(char_t c) const noexcept
  {
    static constexpr std::array ops{
      '+', '-', '*', '/'
    };

    static constexpr auto beg = ops.begin();
    static constexpr auto end = ops.end();
    return std::find(beg, end, c) != end;
  }
  bool lex::is_separator(char_t c) const noexcept
  {
    return is_blank(c) ||
           is_operator(c);
  }
  bool lex::is_blank(char_t c) const noexcept
  {
    return utils::eq_any(c, ' ', '\n', '\t', '\f', '\v', '\r');
  }

}