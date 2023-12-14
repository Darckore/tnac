//
// Symbol
//

#pragma once
#include "sema/sym/sym_scope.hpp"
#include "src_mgr/source_location.hpp"

namespace tnac::semantics
{
  //
  // Symbol kinds
  //
  enum class sym_kind : std::uint8_t
  {
    Variable,
    Parameter,
    Function,
    Module,
    ScopeRef,
    Deferred
  };


  //
  // Base symbol information object. Holds information on an entity
  //
  class symbol
  {
  public:
    using kind   = sym_kind;
    using name_t = string_t;
    using loc_t  = src::loc_wrapper;
    using enum sym_kind;

  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(symbol);

    virtual ~symbol() noexcept;

  protected:
    symbol(kind k, name_t name, scope& owner, loc_t loc) noexcept;

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
    // const version
    //
    const scope& owner_scope() const noexcept;

    //
    // Returns the owner scope
    //
    scope& owner_scope() noexcept;

    //
    // Checks whether the symbol is in the given scope
    //
    bool is_in_scope(const scope& other) const noexcept;

    //
    // Returns the entity name
    //
    string_t name() const noexcept;

    //
    // Returns the source location where this symbol is declared
    //
    loc_t at() const noexcept;

  protected:
    //
    // Overrides the attached location
    //
    void override_location(loc_t loc) noexcept;

  private:
    scope* m_owner{};
    name_t m_name;
    loc_t m_location;
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