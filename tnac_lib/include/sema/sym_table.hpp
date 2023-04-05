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
  class sym_table
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
    // Inserts a variable
    //
    variable& add_variable(ast::decl& decl, scope_ptr parent) noexcept;

    //
    // Looks for a symbol starting from the specified scope
    //
    sym_ptr lookup(string_t name, scope_ptr parent) noexcept;

  private:
    scope_map* lookup(string_t name) noexcept;

    sym_ptr lookup(scope_map* scopes, scope_ptr parent) noexcept;

    scope_map& make_name(string_t name) noexcept;

    sym_ptr& make_symbol(scope_map& scopes, scope_ptr parent) noexcept;

    template <detail::sym S, typename ...Args>
    auto alloc_sym(Args&& ...args) noexcept
    {
      auto&& res = m_symbols.emplace_back(new S{ std::forward<Args>(args)... });
      return sym_cast<S>(res.get());
    }

    template <detail::sym S, typename ...Args>
    auto make_symbol(string_t name, scope_ptr parent, Args&& ...args) noexcept
    {
      auto&& sym = make_symbol(make_name(name), parent);
      if (sym)
        return sym_cast<S>(sym);

      auto res = alloc_sym<S>(std::forward<Args>(args)...);
      sym = res;
      return res;
    }

  private:
    name_map m_names;
    scope_store m_scopes;
    sym_store m_symbols;
  };
}