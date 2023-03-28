//
// Token
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
      return is_any(Plus, Minus, Asterisk, Slash, Assign);
    }

    constexpr auto is_eol() const noexcept
    {
      return is(Eol);
    }

    constexpr auto is_literal() const noexcept
    {
      return is_any(IntBin, IntOct, IntDec, IntHex, Float);
    }

    constexpr auto is_identifier() const noexcept
    {
      return is(Identifier);
    }

    bool operator==(const token&) const noexcept = default;

    string_t m_value;
    kind m_kind{ Error };
  };
}