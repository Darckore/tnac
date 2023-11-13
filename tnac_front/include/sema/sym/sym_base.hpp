//
// Symbol
//

#pragma once
#include "sema/sym/sym_scope.hpp"

namespace tnac::semantics
{
  //
  // Symbol kinds
  //
  enum class sym_kind : std::uint8_t
  {
    Variable,
    Parameter,
    Function
  };


  //
  // Base symbol information object. Holds information on an entity
  //
  class symbol
  {
  public:
    using kind   = sym_kind;
    using name_t = string_t;
    using enum sym_kind;

  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(symbol);

    virtual ~symbol() noexcept;

  protected:
    symbol(kind k, name_t name, const scope& owner) noexcept;

  public:
    //
    // Returns the symbol kind
    //
    kind what() const noexcept;

    //
    // Checks whether the symbol is of the specified kind
    //
    bool is(kind k) const noexcept;

    //
    // Checks whether the symbol is of one of the specified kinds
    //
    template <typename... KINDS> requires(utils::all_same<kind, KINDS...>)
      auto is_any(KINDS... kinds) const noexcept
    {
      return ((is(kinds)) || ...);
    }

    //
    // Returns the owner scope
    //
    const scope& owner_scope() const noexcept;

    //
    // Returns the entity name
    //
    string_t name() const noexcept;

  private:
    const scope* m_owner{};
    name_t m_name;
    kind m_kind{};
  };

  namespace detail
  {
    template <typename T>
    concept sym = std::derived_from<T, symbol>;
  }

  inline auto get_id(const symbol& sym) noexcept
  {
    return sym.what();
  }
}