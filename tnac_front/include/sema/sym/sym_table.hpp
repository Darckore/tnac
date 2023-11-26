//
// Symbol table
//

#pragma once
#include "sema/sym/symbols.hpp"

namespace tnac::semantics
{
  template <detail::sym S>
  using sym_collection = std::unordered_map<const scope*, std::vector<const S*>>;

  //
  // Iterator which allows going through scopes associated with a collection
  // of certain symbol types
  //
  template <detail::sym S>
  class scope_iter final
  {
  public:
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using collection_type = sym_collection<S>;
    using value_type      = collection_type::mapped_type;
    using underlying_iter = collection_type::const_iterator;
    using scope_ptr       = collection_type::key_type;
    using pointer         = const value_type*;
    using reference       = const value_type&;

  public:
    CLASS_SPECIALS_NODEFAULT(scope_iter);

    scope_iter(underlying_iter it) noexcept :
      m_it{ it }
    {}

    bool operator==(const scope_iter&) const noexcept = default;

    scope_iter& operator++() noexcept
    {
      ++m_it;
      return *this;
    }

    scope_iter operator++(int) noexcept
    {
      auto&& self = *this;
      auto copy = self;
      ++self;
      return copy;
    }

    reference operator*() const noexcept
    {
      return m_it->second;
    }

    pointer operator->() const noexcept
    {
      return &(operator*());
    }

    scope_ptr scope() const noexcept
    {
      return m_it->first;
    }

  private:
    underlying_iter m_it;
  };

  //
  // Iterable container for a collection of symbols
  //
  template <detail::sym S>
  class sym_container final
  {
  public:
    using iterator = scope_iter<S>;

  public:
    CLASS_SPECIALS_NODEFAULT(sym_container);

    sym_container(iterator beg, iterator end) noexcept :
      m_beg{ beg },
      m_end{ end }
    {}

  public:
    auto begin() const noexcept
    {
      return m_beg;
    }
    auto end() const noexcept
    {
      return m_end;
    }

  private:
    iterator m_beg;
    iterator m_end;
  };

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
    using name_t = symbol::name_t;

    using scope_ptr   = scope*;
    using scope_owner = owning_ptr<scope>;
    using scope_store = entity_list<scope_owner>;

    using sym_ptr   = symbol*;
    using sym_owner = owning_ptr<symbol>;
    using sym_store = entity_list<sym_owner>;

    using scope_map = table<scope_ptr, sym_ptr>;
    using name_map  = table<name_t, scope_map>;

    using var_collection    = sym_collection<variable>;
    using var_container     = sym_container<variable>;
    using func_collection   = sym_collection<function>;
    using func_container    = sym_container<function>;
    using module_collection = sym_collection<module_sym>;
    using module_container  = sym_container<module_sym>;

    using loc_t = symbol::loc_t;
    using func_params = function::param_list;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(sym_table);

    ~sym_table() noexcept;
    sym_table() noexcept;

  public:
    //
    // Inserts a new scope inside the one specified as the parent
    //
    scope& add_scope(scope_ptr parent, scope_kind kind) noexcept;

    //
    // Inserts a variable to the specified scope
    //
    variable& add_variable(name_t name, scope_ptr parent, loc_t loc) noexcept;

    //
    // Inserts a parameter to the specified scope
    //
    parameter& add_parameter(name_t name, scope_ptr parent, loc_t loc) noexcept;

    //
    // Inserts a function to the specified scope
    //
    function& add_function(name_t name, scope_ptr parent, func_params params, loc_t loc, scope& owned) noexcept;

    //
    // Inserts a module to the specified scope
    //
    module_sym& add_module(name_t name, scope_ptr parent, loc_t loc, scope& owned) noexcept;

    //
    // Inserts a scope reference to the specified scope
    //
    scope_ref& add_scope_ref(name_t name, scope_ptr parent, loc_t loc, scope& referenced) noexcept;

    //
    // Inserts a deferred symbol to the specified scope
    //
    deferred& add_deferred_sym(name_t name, scope_ptr parent, loc_t loc) noexcept;

    //
    // Looks for a symbol starting from the specified scope
    //
    sym_ptr lookup(name_t name, scope_ptr parent) noexcept;

    //
    // Looks up a symbol in the given scope only
    //
    sym_ptr scoped_lookup(name_t name, scope_ptr parent) noexcept;

    //
    // Returns an iterable collection of all declared variables
    //
    var_container vars() const noexcept;

    //
    // Returns an iterable collection of all declared functions
    //
    func_container funcs() const noexcept;

    //
    // Returns an iterable collection of all declared modules
    //
    module_container modules() const noexcept;

  private:
    //
    // Searches for the given name in the name map and returns a pointer to
    // the table of scopes where at least one entity with such name is defined
    // Returns nullptr if the name doesn't exist
    //
    scope_map* lookup(name_t name) noexcept;

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
    // entities with such name exist
    //
    scope_map& make_name(name_t name) noexcept;

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
    auto make_symbol(name_t name, scope_ptr parent, Args&& ...args) noexcept
    {
      auto&& sym = make_symbol(make_name(name), parent);
      if (sym)
      {
        UTILS_ASSERT(sym->is(utils::type_to_id_v<S>));
        return utils::try_cast<S>(sym);
      }

      auto res = alloc_sym<S>(*parent, name, std::forward<Args>(args)...);
      sym = res;
      return res;
    }

    //
    // Stores a symbol in the corresponding collection
    // This is mostly needed for enumeration of everything in the given scope
    //
    template <detail::sym S, typename Collection>
      requires (std::same_as<Collection, sym_collection<S>>)
    void store_sym(S& sym, Collection& store) noexcept
    {
      auto parent = &sym.owner_scope();
      auto&& newIt = store[parent];
      newIt.emplace_back(&sym);
    }

    //
    // Stores a variable in the collection which holds all variables in the program
    // along with their scopes
    //
    void store_variable(variable& var) noexcept;

    //
    // Stores a function in the collection which holds all functions in the program
    // along with their scopes
    //
    void store_function(function& func) noexcept;

    //
    // Stores a module in the collection which holds all modules in the program
    // along with their scopes
    //
    void store_module(module_sym& mod) noexcept;

  private:
    name_map m_names;
    scope_store m_scopes;
    sym_store m_symbols;
    var_collection m_vars;
    func_collection m_funcs;
    module_collection m_modules;
  };
}