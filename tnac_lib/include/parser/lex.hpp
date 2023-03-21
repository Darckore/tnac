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

    constexpr auto is_eol() const noexcept
    {
      return is(Eol);
    }

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
    // Checks whether the buffer is at an end
    //
    bool good() const noexcept;

  private:    
    value_type m_buf;
    buf_iterator m_from{};
    buf_iterator m_to{};
  };
}