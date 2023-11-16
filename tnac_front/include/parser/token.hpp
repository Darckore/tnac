//
// Token
//

#pragma once
#include "src_mgr/source_location.hpp"

namespace tnac
{
  //
  // Different kinds of tokens
  //
  enum class tok_kind : std::uint8_t
  {
    Error,
    Eol,

    // Expr separator
    ExprSep,

    // Numeric tokens
    IntBin,
    IntOct,
    IntDec,
    IntHex,
    Float,

    // String literal
    String,

    // Operators
    Dot,
    Exclamation,
    Question,
    Plus,
    Minus,
    Asterisk,
    Slash,
    Percent,
    Assign,
    Tilde,
    Amp,
    Hat,
    Pipe,
    Pow,
    Root,

    Eq,
    NotEq,

    Less,
    LessEq,
    Greater,
    GreaterEq,

    LogAnd,
    LogOr,

    Arrow,

    // Punctuation
    ParenOpen,
    ParenClose,
    Comma,
    Semicolon,
    CurlyOpen,
    CurlyClose,
    BracketOpen,
    BracketClose,

    // Identifiers
    Identifier,
    Command,

    // Keywords
    KwResult,
    KwFunction,
    KwComplex,
    KwFraction,
    KwInt,
    KwFloat,
    KwBool,
    KwRet,
    KwTrue,
    KwFalse,
    KwI,
    KwE,
    KwPi,
    KwEntry,
    KwImport
  };

  //
  // Token used to represent lexemes
  //
  class token final
  {
  public:
    using loc  = src::loc_wrapper;
    using kind = tok_kind;
    using enum tok_kind;

  public:
    CLASS_SPECIALS_NODEFAULT(token);

    token(string_t value, kind k, loc location) noexcept :
      m_value{ value },
      m_loc{ location },
      m_kind{ k }
    {}

    token(string_t value, kind k) noexcept :
      token{ value, k, src::location::dummy().record() }
    {}

    bool operator==(const token& other) const noexcept
    {
      return m_kind == other.m_kind && m_value == other.m_value;
    }

  public:
    token get_after() const noexcept
    {
      const auto delta = static_cast<src::location::line_pos>(m_value.length());
      auto location = *m_loc;
      location.incr_column_by(delta);
      return { m_value, Error, location.record() };
    }

    loc at() const noexcept
    {
      return m_loc;
    }

    string_t value() const noexcept
    {
      return m_value;
    }

    kind what() const noexcept
    {
      return m_kind;
    }

    auto is(kind k) const noexcept
    {
      return what() == k;
    }

    template <typename... KINDS> requires(utils::all_same<kind, KINDS...>)
    auto is_any(KINDS... kinds) const noexcept
    {
      return ((is(kinds)) || ...);
    }

    auto is_eol() const noexcept
    {
      return is(Eol);
    }

    auto is_keyword() const noexcept
    {
      return is_any(KwResult,
                    KwFunction,
                    KwComplex,
                    KwFraction,
                    KwInt,
                    KwFloat,
                    KwBool,
                    KwRet,
                    KwTrue,
                    KwFalse,
                    KwI,
                    KwE,
                    KwPi,
                    KwEntry,
                    KwImport);
    }

    auto is_literal() const noexcept
    {
      return is_any(IntBin, IntOct, IntDec, IntHex, Float,
                    KwTrue, KwFalse, KwI, KwPi, KwE);
    }

    auto is_identifier() const noexcept
    {
      return is(Identifier);
    }

  private:
    string_t m_value;
    loc m_loc;
    kind m_kind{ Error };
  };
}