//
// Symbol table
//

#pragma once

namespace tnac::ast
{
  class scope;
  class decl;
}

namespace tnac::semantics
{
  class symbol;
  struct scope;
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

    using sym_ptr   = const symbol*;
    using sym_owner = owning_ptr<symbol>;
    using sym_store = entity_list<sym_owner>;
    using sym_list  = entity_list<sym_ptr>;

    using scope_map = table<scope_ptr, sym_list>;
    using name_map  = table<string_t, scope_map>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(sym_table);

    ~sym_table() noexcept;
    sym_table() noexcept;

  public:
    //
    // Inserts a new scope inside the one specified as the parent
    //
    scope& add_scope(const ast::scope* node, const scope* parent) noexcept;

  private:
    name_map m_names;
    scope_store m_scopes;
    sym_store m_symbols;
  };
}