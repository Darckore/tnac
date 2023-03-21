//
// Lexer
//

#pragma once

namespace tnac
{
  //
  // Token used to represent lexemes
  //
  struct token
  {
    enum class kind : std::uint8_t
    {
      Error,
      Eol,
      
      // Numeric tokens
      IntDec,
      Float,

      // Operators
      Plus,
      Minus,
      Asterisk,
      Slash
    };

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

    string_t m_value;
    kind m_kind{ Error };
  };

  using tok_kind = token::kind;

  //
  // Lexer for the input parser
  //
  class lex
  {
  public:
    using value_type = string_t;
    using buf_iterator = string_t::iterator;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(lex);

    ~lex() noexcept = default;
    lex() = default;

    //
    // Non-owning string is passed here
    // Lifetime must be maintained outside
    //
    explicit lex(string_t buf) noexcept;

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

  private:
    //
    // Consumes the current character sequence between from an to iterators,
    // and advances the from iterator to the next non-blank character
    //
    token consume(tok_kind kind) noexcept;

    //
    // Moves forward until a separator is encountered
    //
    void ffwd() noexcept;

    //
    // Tries to parse a number
    //
    token number() noexcept;

    //
    // Tries to parse an operator
    //
    token op() noexcept;

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
    // Checks whether the given character is a digit
    //
    bool is_digit(char_t c) const noexcept;

    //
    // Checks whether the given character belongs to an operator
    //
    bool is_operator(char_t c) const noexcept;

    //
    // Checks whether the given character is a non-blank separator
    //
    bool is_separator(char_t c) const noexcept;

    //
    // Checks whether the given character is blank
    //
    bool is_blank(char_t c) const noexcept;

  private:    
    value_type m_buf;
    buf_iterator m_from{};
    buf_iterator m_to{};
  };
}