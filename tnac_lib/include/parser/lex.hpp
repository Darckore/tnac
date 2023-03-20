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
    enum class kind
    {
      Error,
      Eol,
      
      // Numeric tokens
      Int,
      Float,

      // Operators
      Plus,
      Minus,
      Mul,
      Div
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
      return is_any(Plus, Minus, Mul, Div);
    }

    string_t m_pos;
    kind m_kind{ Error };
  };

  //
  // Lexer for the input parser
  //
  class lex
  {
  public:
    CLASS_SPECIALS_NONE_CUSTOM(lex);

    ~lex() noexcept = default;
    lex() = default;

    explicit lex(buf_t buf) noexcept;

  public:
    //
    // Feeds an input string to the lexer
    // replacing the currently held buffer
    //
    void feed(buf_t buf) noexcept;

    //
    // Obtains the next token from the input
    //
    token next() noexcept;

  private:
    //
    // Checks whether the buffer is at an end
    //
    bool good() const noexcept;

  private:
    using buf_it = buf_t::iterator;
    
    buf_t m_buf;
    buf_it m_from{};
    buf_it m_to{};
  };
}