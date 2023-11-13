//
// Symbol
//

#pragma once

namespace tnac::semantics
{
  //
  // The kind of the scope
  //
  enum class scope_kind : std::uint8_t
  {
    Global,
    Module,
    Function,
    Block
  };

  //
  // Represents a scope
  // We it need to keep track of where symbols are declared to disambiguate
  // between entities having the same name
  //
  class scope final
  {
  public:
    using depth_t = std::uint32_t;
    using enum scope_kind;

  public:
    CLASS_SPECIALS_NONE(scope);

    ~scope() noexcept;

    scope(scope* encl, scope_kind kind) noexcept;

  public:
    //
    // Returns a pointer to the enclosing scope
    // 
    // const version
    //
    const scope* enclosing() const noexcept;

    //
    // Returns a pointer to the enclosing scope
    //
    scope* enclosing() noexcept;

    //
    // Climbs to the nearest function or module parent
    // 
    // const version
    //
    const scope* encl_skip_internal() const noexcept;

    //
    // Climbs to the nearest function or module parent
    //
    scope* encl_skip_internal() noexcept;

    //
    // Returns the scope's depth
    //
    depth_t depth() const noexcept;

    //
    // Returns the kind of the current scope
    //
    scope_kind kind() const noexcept;

    //
    // Checks if the scope is of the given kind
    //
    bool is(scope_kind k) const noexcept;

    //
    // Checks if the scope is of one of the given kinds
    //
    template <typename... KINDS> requires(utils::all_same<scope_kind, KINDS...>)
    auto is_any(KINDS... kinds) const noexcept
    {
      return ((is(kinds)) || ...);
    }

    //
    // Checks if the scope is global or module
    //
    bool is_top_level() const noexcept;

    //
    // Checks if the scope represents a function body
    //
    bool is_function() const noexcept;

    //
    // Checks if the scope is internal (i.e., condition body)
    //
    bool is_internal() const noexcept;

  private:
    scope* m_enclosing{};
    depth_t m_depth{};
    scope_kind m_scopeKind;
  };
}