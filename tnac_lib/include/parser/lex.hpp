//
// Lexer
//

#pragma once

namespace tnac
{
  //
  // Different kinds of tokens
  //
  enum class tok_kind : std::uint8_t
  {
#include "general/token_kinds.inl"
  };

  //
  // Token used to represent lexemes
  //
  struct token
  {
    using kind = tok_kind;
    using enum kind;

    constexpr auto is(kind k) const noexcept
    {
      return m_kind == k;
    }

    template <typename... KINDS> requires(utils::detail::all_same<kind, KINDS...>)
    constexpr auto is_any(KINDS... kinds) const noexcept
    {
      return ((is(kinds)) || ...);
    }

    constexpr auto is_operator() const noexcept
    {
      return is_any(Plus, Minus, Asterisk, Slash);
    }

    constexpr auto is_eol() const noexcept
    {
      return is(Eol);
    }

    bool operator==(const token&) const noexcept = default;

    string_t m_value;
    kind m_kind{ Error };
  };

  //
  // Lexer for the input parser
  //
  class lex
  {
  public:
    using value_type = string_t;
    using buf_iterator = string_t::iterator;

    using token_opt = std::optional<token>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(lex);

    ~lex() noexcept = default;
    lex() = default;

  public:
    //
    // Feeds an input string to the lexer
    // replacing the currently held buffer
    //
    void feed(string_t buf) noexcept;

    //
    // Obtains the next token from the input
    //
    token next() noexcept;

    //
    // Previews the next token
    //
    const token& peek() noexcept;

  private:
    //
    // Consumes the current character sequence between from an to iterators,
    // and advances the from iterator to the next non-blank character
    //
    const token& consume(tok_kind kind) noexcept;

    //
    // Moves forward until a separator is encountered
    //
    void ffwd() noexcept;

    //
    // Tries to parse a number
    //
    const token& number() noexcept;

    //
    // Tries to parse a binary integer
    //
    const token& bin_number() noexcept;

    //
    // Tries to parse a hex integer
    //
    const token& hex_number() noexcept;

    //
    // Implementation for bin and hex
    //
    const token& hex_bin_impl(bool isHex) noexcept;

    //
    // Tries to parse a float or decimal int
    //
    const token& decimal_number(bool leadingZero) noexcept;

    //
    // Checks whether the is a sequence of digits following the current position
    // Moves the to iterator past the last digit
    // 
    // Fails if the digit sequence doesn't end with a separator or a '.'
    //
    bool digit_seq(unsigned base) noexcept;

    //
    // Tries to parse an operator
    //
    const token& op() noexcept;

    //
    // Previews the next character
    //
    char_t peek_char() const noexcept;

    //
    // Advances the iterator one character forward
    //
    void advance() noexcept;

    //
    // Checks whether the buffer is at an end
    //
    bool good() const noexcept;

    //
    // Clears the preview token
    //
    void clear_preview() noexcept;

  private:
    value_type m_buf;
    buf_iterator m_from{};
    buf_iterator m_to{};
    token_opt m_preview{};
  };
}