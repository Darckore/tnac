//
// Symbol table
//

#pragma once
#include "sema/symbol.hpp"

namespace tnac::ast
{
  class scope;
  class decl;
}

namespace tnac::semantics
{
  //
  // Symbol table. Stores symbols associated to names in respect with their scopes
  //
  class sym_table final
  {
  private:
    template <typename T>
    using owning_ptr = std::unique_ptr<T>;

    template <typename T>
    using entity_list = std::vector<T>;

    template <typename K, typename V>
    using table = std::unordered_map<K, V>;

  public:
    using scope_ptr   = const scope*;
    using scope_owner = owning_ptr<scope>;
    using scope_store = entity_list<scope_owner>;

    using sym_ptr   = symbol*;
    using sym_owner = owning_ptr<symbol>;
    using sym_store = entity_list<sym_owner>;

    using scope_map = table<scope_ptr, sym_ptr>;
    using name_map  = table<string_t, scope_map>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(sym_table);

    ~sym_table() noexcept;
    sym_table() noexcept;

  public:
    //
    // Inserts a new scope inside the one specified as the parent
    //
    const scope& add_scope(const ast::scope* node, scope_ptr parent) noexcept;

    //
    // Inserts a variable to the specified scope
    //
    variable& add_variable(ast::decl& decl, scope_ptr parent) noexcept;

    //
    // Inserts a parameter to the specified scope
    //
    parameter& add_parameter(ast::decl& decl, scope_ptr parent) noexcept;

    //
    // Inserts a function to the specified scope
    //
    function& add_function(ast::decl& decl, scope_ptr parent) noexcept;

    //
    // Looks for a symbol starting from the specified scope
    //
    sym_ptr lookup(string_t name, scope_ptr parent) noexcept;

    //
    // Looks up a symbol in the given scope only
    //
    sym_ptr scoped_lookup(string_t name, scope_ptr parent) noexcept;

  private:
    //
    // Searches for the given name in the name map and returns a pointer to
    // the table of scopes where at least one entity with such name is defined
    // Returns nullptr if the name doesn't exist
    //
    scope_map* lookup(string_t name) noexcept;

    //
    // Searches for the provided scope in the scope table. On failure, repeats the search
    // by using the scope's parent and so on
    // This continues until the first match, or if the topmost null scope is reached
    // 
    // If the scope is found, returns a pointer to the symbol defined in it
    // Otherwise, returns nullptr
    //
    sym_ptr lookup(scope_map* scopes, scope_ptr parent, bool current) noexcept;

    //
    // Adds the given name to the name map and returns an empty scope table
    // If the name already exists, returns the scopes where
    // an entities with such name exist
    //
    scope_map& make_name(string_t name) noexcept;

    //
    // Adds a symbol to the scope table and returns a reference to it
    //
    sym_ptr& make_symbol(scope_map& scopes, scope_ptr parent) noexcept;

    //
    // Allocates a new symbol of the specified type
    //
    template <detail::sym S, typename ...Args>
    auto alloc_sym(Args&& ...args) noexcept
    {
      auto&& res = m_symbols.emplace_back(new S{ std::forward<Args>(args)... });
      return utils::try_cast<S>(res.get());
    }

    //
    // Tries to add a symbol to the given scope and returns a pointer to it
    // If the scope already has a symbol of the same, returns a pointer to it
    // If the existing symbol has a different type, returns nullptr
    //
    template <detail::sym S, typename ...Args>
    auto make_symbol(string_t name, scope_ptr parent, Args&& ...args) noexcept
    {
      auto&& sym = make_symbol(make_name(name), parent);
      if (sym)
        return utils::try_cast<S>(sym);

      auto res = alloc_sym<S>(*parent, std::forward<Args>(args)...);
      sym = res;
      return res;
    }

  private:
    name_map m_names;
    scope_store m_scopes;
    sym_store m_symbols;
  };
}