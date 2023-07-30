//
// Lexer
//

#pragma once
#include "parser/token.hpp"
#include "src_mgr/source_location.hpp"

namespace tnac
{
  //
  // Lexer for the input parser
  //
  class lex final
  {
  public:
    using value_type = string_t;
    using buf_iterator = string_t::iterator;

    using token_opt = std::optional<token>;

    using loc = src::location;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(lex);

    ~lex() noexcept;
    lex() noexcept;

  public:
    //
    // Feeds an input string to the lexer
    // replacing the currently held buffer
    //
    void operator()(string_t buf) noexcept;

    //
    // Obtains the next token from the input
    //
    token next() noexcept;

    //
    // Previews the next token
    //
    const token& peek() noexcept;

    //
    // Accepts a source location object for the current source file
    //
    void attach_loc(loc& srcLoc) noexcept;

    //
    // Drops the previously saved location
    //
    void detach_loc() noexcept;

  private:
    //
    // Consumes the current character sequence between from an to iterators,
    // and advances the from iterator to the next non-blank character
    //
    const token& consume(tok_kind kind) noexcept;

    //
    // Retrieves the string of characters between the from and to iterators
    //
    string_t read_str() const noexcept;

    //
    // Moves forward until a separator is encountered
    //
    void ffwd() noexcept;

    //
    // Skips comments
    // Returns false if EOL is reached
    //
    bool skip_comment() noexcept;

    //
    // Tries to parse a string
    //
    const token& string() noexcept;

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
    // Checks whether there is a sequence of digits following the current position
    // Moves the to iterator past the last digit
    // 
    // Fails if the digit sequence doesn't end with a separator or a '.'
    //
    bool digit_seq(unsigned base) noexcept;

    //
    // Tries to parse a keyword
    // Fails if the keyword name is unknown
    //
    const token& keyword() noexcept;

    //
    // Tries to parse an identifier
    //
    const token& identifier() noexcept;

    //
    // Checks whether there is a sequence of id chars following the current position
    // Moves the to iterator past the last character
    // 
    // Fails is the sequence doesn't end with a separator
    //
    bool id_seq() noexcept;

    //
    // Tries to parse an operator
    //
    const token& op() noexcept;

    //
    // Checks if the next char is the one expected and returns
    // the 'expected' token kind if it is
    // Otherwise, returns 'fallback'
    // 
    // Advances the buffer iterator if successful
    //
    tok_kind try_next(char_t expChar, tok_kind expected, tok_kind fallback) noexcept;

    //
    // Tries to parse punctuation
    //
    const token& punct() noexcept;

    //
    // Previews the next character
    //
    char_t peek_char() const noexcept;

    //
    // Advances the iterator one character forward
    //
    void advance() noexcept;

    //
    // Sets the sequence begin iterator to the end
    //
    void collapse() noexcept;

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
    loc* m_srcLocation{};
  };
}