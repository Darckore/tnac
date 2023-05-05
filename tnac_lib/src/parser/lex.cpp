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
      constexpr auto is_expr_separator(char_t c) noexcept
      {
        return c == ':';
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
          '+', '-', '~', '*', '/', '%', '='
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
      constexpr auto is_paren_open(char_t c) noexcept
      {
        return c == '(';
      }
      constexpr auto is_paren_close(char_t c) noexcept
      {
        return c == ')';
      }
      constexpr auto is_paren(char_t c) noexcept
      {
        return is_paren_open(c) || is_paren_close(c);
      }
      constexpr auto is_comma(char_t c) noexcept
      {
        return c == ',';
      }
      constexpr auto is_separator(char_t c) noexcept
      {
        return is_expr_separator(c) ||
               is_comma(c) ||
               is_paren(c) ||
               is_blank(c) ||
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

      constexpr auto is_alpha(char_t c) noexcept
      {
        return utils::in_range(to_lower(c), 'a', 'z');
      }
      constexpr auto is_underscore(char_t c) noexcept
      {
        return c == '_';
      }
      constexpr auto is_id_start(char_t c) noexcept
      {
        return is_alpha(c);
      }
      constexpr auto is_id_char(char_t c) noexcept
      {
        return is_underscore(c) ||
               is_id_start(c) ||
               is_digit(c, 10);
      }
      constexpr auto is_command_start(char_t c) noexcept
      {
        return c == '#';
      }
      
      constexpr auto is_single_quote(char_t c) noexcept
      {
        return c == '\'';
      }

      constexpr auto is_any_name_start(char_t c) noexcept
      {
        return is_id_start(c) ||
               is_underscore(c) ||
               is_command_start(c);
      }

      auto lookup_keyword(string_t name) noexcept
      {
        using kw_map = std::unordered_map<string_t, tok_kind>;

        static const kw_map keywords{
          { "result",   tok_kind::KwResult },
          { "complex",  tok_kind::KwComplex },
          { "fraction", tok_kind::KwFraction },
        };

        constexpr auto err = tok_kind::Error;
        if (name.empty() || !is_underscore(name.front()))
          return err;

        name.remove_prefix(1);
        if (auto foundIt = keywords.find(name); foundIt != keywords.end())
          return foundIt->second;

        return err;
      }
    }
  }


  // Public members

  void lex::operator()(string_t buf) noexcept
  {
    m_buf = utils::ltrim(buf);
    m_from = m_buf.begin();
    m_to   = m_from;
    clear_preview();
  }

  token lex::next() noexcept
  {
    auto tok = peek();
    clear_preview();
    return tok;
  }

  const token& lex::peek() noexcept
  {
    using enum tok_kind;

    if (m_preview)
      return *m_preview;

    if (!good())
      return consume(tok_kind::Eol);

    const auto next = peek_char();

    if (detail::is_single_quote(next))
    {
      return string();
    }

    if (detail::is_any_name_start(next))
    {
      return identifier();
    }

    if (detail::is_digit(next))
    {
      return number();
    }

    if (detail::is_operator(next))
    {
      return op();
    }

    if (detail::is_separator(next))
    {
      return punct();
    }

    return consume(Error);
  }


  // Private members

  const token& lex::consume(tok_kind kind) noexcept
  {
    if (kind == tok_kind::Error)
      ffwd();

    if (kind == tok_kind::Command)
      ++m_from; // Skipping the leading '#'

    auto tokVal = (kind != tok_kind::Eol) ? read_str() : string_t{};

    // Removing leading and trailing ' of a string
    if (kind == tok_kind::String)
    {
      tokVal.remove_prefix(1);
      tokVal.remove_suffix(1);
    }

    token res{ .m_value{ tokVal }, .m_kind{ kind } };

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
    m_preview = res;
    return *m_preview;
  }

  string_t lex::read_str() const noexcept
  {
    return { m_from, m_to };
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

  const token& lex::string() noexcept
  {
    advance();
    while (good())
    {
      const auto next = peek_char();
      if (detail::is_single_quote(next))
      {
        advance();
        return consume(tok_kind::String);
      }

      advance();
    }

    return consume(tok_kind::Error);
  }

  const token& lex::number() noexcept
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
  const token& lex::bin_number() noexcept
  {
    return hex_bin_impl(false);
  }
  const token& lex::hex_number() noexcept
  {
    return hex_bin_impl(true);
  }
  const token& lex::hex_bin_impl(bool isHex) noexcept
  {
    using enum tok_kind;
    advance();

    const auto base = isHex ? 16u : 2u;
    const auto kind = isHex ? IntHex : IntBin;

    if (!digit_seq(base))
      return consume(Error);

    return detail::is_separator(peek_char()) ? consume(kind) : consume(Error);
  }

  const token& lex::decimal_number(bool leadingZero) noexcept
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

  const token& lex::keyword() noexcept
  {
    const auto kw = detail::lookup_keyword(read_str());
    return consume(kw);
  }

  const token& lex::identifier() noexcept
  {
    using enum tok_kind;
    const auto first = peek_char();

    if (!detail::is_id_start(first))
    {
      advance();
    }

    if (!id_seq())
      return consume(Error);

    if (detail::is_underscore(first))
    {
      return keyword();
    }
    else if (detail::is_command_start(first))
    {
      return consume(Command);
    }

    return consume(Identifier);
  }

  bool lex::id_seq() noexcept
  {
    bool ok = false;
    while (good())
    {
      const auto next = peek_char();
      if (detail::is_id_char(next))
      {
        ok = true;
        advance();
        continue;
      }

      if (detail::is_separator(next))
        break;

      ok = false;
      break;
    }

    return ok;
  }

  const token& lex::op() noexcept
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

    case '~':
      resKind = Tilde;
      break;

    case '*':
      resKind = Asterisk;
      break;

    case '/':
      resKind = Slash;
      break;

    case '%':
      resKind = Percent;
      break;

    case '=':
      resKind = Assign;
      break;

    default:
      resKind = Error;
      break;
    }

    return consume(resKind);
  }

  const token& lex::punct() noexcept
  {
    using enum tok_kind;
    const auto next = peek_char();
    advance();

    auto resKind = Eol;
    switch (next)
    {
    case ':':
      resKind = ExprSep;
      break;

    case '(':
      resKind = ParenOpen;
      break;

    case ')':
      resKind = ParenClose;
      break;

    case ',':
      resKind = Comma;
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

  void lex::clear_preview() noexcept
  {
    m_preview.reset();
  }

}