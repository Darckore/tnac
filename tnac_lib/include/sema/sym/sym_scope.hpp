//
// Symbol
//

#pragma once

namespace tnac
{
  class sema;
}

namespace tnac::semantics
{
  class symbol;
  class function;
  class module_sym;
  class scope_ref;
}

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
    Block,
    Deferred
  };

  //
  // Represents a scope
  // We it need to keep track of where symbols are declared to disambiguate
  // between entities having the same name
  //
  class scope final
  {
  private:
    friend class tnac::sema;

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
    const scope* enclosing() const noexcept;

    //
    // Returns a pointer to the enclosing scope
    //
    scope* enclosing() noexcept;

    //
    // Climbs to the nearest function or module parent
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
    // Checks if the scope represents a module
    //
    bool is_module() const noexcept;

    //
    // Checks if the scope represents a function body
    //
    bool is_function() const noexcept;

    //
    // Checks if the scope is internal (i.e., condition body)
    //
    bool is_internal() const noexcept;

    //
    // Checks if the scope holds deferred symbols
    //
    bool is_deferred() const noexcept;

    //
    // Checks whether a symbol is attached
    //
    bool has_sym() const noexcept;

    //
    // Returns a pointer to the attached symbol
    //
    const symbol* sym() const noexcept;

    //
    // Returns a pointer to the attached symbol
    //
    symbol* sym() noexcept;

    //
    // Attempts to cast the attached symbol to function
    //
    const function* to_func() const noexcept;

    //
    // Attempts to cast the attached symbol to function
    //
    function* to_func() noexcept;

    //
    // Casts the attached symbol to function without checking if one is attached
    //
    const function& func() const noexcept;

    //
    // Casts the attached symbol to function without checking if one is attached
    //
    function& func() noexcept;

    //
    // Attempts to cast the attached symbol to module
    //
    const module_sym* to_module() const noexcept;

    //
    // Attempts to cast the attached symbol to module
    //
    module_sym* to_module() noexcept;

    //
    // Casts the attached symbol to module without checking if one is attached
    //
    const module_sym& mod() const noexcept;

    //
    // Casts the attached symbol to module without checking if one is attached
    //
    module_sym& mod() noexcept;

    //
    // Attempts to cast the attached symbol to callable (module or function)
    //
    const function* to_callable() const noexcept;

    //
    // Attempts to cast the attached symbol to callable (module or function)
    //
    function* to_callable() noexcept;

    //
    // Attempts to cast the attached symbol to scope reference
    //
    const scope_ref* to_scope_ref() const noexcept;

    //
    // Attempts to cast the attached symbol to scope reference
    //
    scope_ref* to_scope_ref() noexcept;

    //
    // Casts the attached symbol to scope reference without checking if one is attached
    //
    const scope_ref& sc_ref() const noexcept;

    //
    // Casts the attached symbol to scope reference without checking if one is attached
    //
    scope_ref& sc_ref() noexcept;

  protected:
    //
    // Attaches a symbol
    //
    void attach_symbol(symbol& sym) noexcept;

  private:
    scope* m_enclosing{};
    symbol* m_sym{};
    depth_t m_depth{};
    scope_kind m_scopeKind;
  };
}