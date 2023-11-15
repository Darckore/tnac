#include "sema/sym/sym_table.hpp"
#include "sema/sym/sym_base.hpp"

namespace tnac::semantics
{
  // Special members

  sym_table::~sym_table() noexcept = default;

  sym_table::sym_table() noexcept = default;

  // Public members

  scope& sym_table::add_scope(scope_ptr parent, scope_kind kind) noexcept
  {
    auto&& insertedScope = m_scopes.emplace_back(std::make_unique<scope>(parent, kind));
    return *insertedScope.get();
  }

  variable& sym_table::add_variable(name_t name, scope_ptr parent, loc_t loc) noexcept
  {
    auto&& newVar = *make_symbol<variable>(name, parent, loc);
    store_variable(newVar);
    return newVar;
  }

  parameter& sym_table::add_parameter(name_t name, scope_ptr parent, loc_t loc) noexcept
  {
    return *make_symbol<parameter>(name, parent, loc);
  }

  function& sym_table::add_function(name_t name, scope_ptr parent, func_params params, loc_t loc) noexcept
  {
    auto&& newFunc = *make_symbol<function>(name, parent, std::move(params), loc);
    store_function(newFunc);
    return newFunc;
  }

  module_sym& sym_table::add_module(name_t name, scope_ptr parent, loc_t loc) noexcept
  {
    auto&& newModule = *make_symbol<module_sym>(name, parent, loc);
    store_module(newModule);
    return newModule;
  }

  sym_table::sym_ptr sym_table::lookup(name_t name, scope_ptr parent) noexcept
  {
    auto scopes = lookup(name);
    return lookup(scopes, parent, false);
  }

  sym_table::sym_ptr sym_table::scoped_lookup(name_t name, scope_ptr parent) noexcept
  {
    auto scopes = lookup(name);
    return lookup(scopes, parent, true);
  }

  sym_table::var_container sym_table::vars() const noexcept
  {
    return { m_vars.begin(), m_vars.end() };
  }

  sym_table::func_container sym_table::funcs() const noexcept
  {
    return { m_funcs.begin(), m_funcs.end() };
  }

  sym_table::module_container sym_table::modules() const noexcept
  {
    return { m_modules.begin(), m_modules.end() };
  }

  // Private members

  sym_table::sym_ptr sym_table::lookup(scope_map* scopes, scope_ptr parent, bool current) noexcept
  {
    sym_ptr res{};
    if (!scopes)
      return res;

    bool reachedFunction{};
    while (parent)
    {
      auto scopeIt = scopes->find(parent);
      if (scopeIt != scopes->end())
      {
        res = scopeIt->second;
        break;
      }
      if (current)
        break;

      if (parent->is_function())
        reachedFunction = true;

      parent = parent->enclosing();
    }

    // Variables and parameters don't leak acros functions
    if (reachedFunction && res && res->is_any(symbol::Variable, symbol::Parameter))
    {
      res = {};
    }

    return res;
  }

  sym_table::scope_map* sym_table::lookup(name_t name) noexcept
  {
    auto nameIt = m_names.find(name);
    if (nameIt == m_names.end())
      return {};

    return &nameIt->second;
  }

  sym_table::scope_map& sym_table::make_name(name_t name) noexcept
  {
    return m_names.emplace(name, scope_map{}).first->second;
  }

  sym_table::sym_ptr& sym_table::make_symbol(scope_map& scopes, scope_ptr parent) noexcept
  {
    return scopes.emplace(parent, sym_ptr{}).first->second;
  }

  void sym_table::store_variable(variable& var) noexcept
  {
    store_sym(var, m_vars);
  }

  void sym_table::store_function(function& func) noexcept
  {
    store_sym(func, m_funcs);
  }

  void sym_table::store_module(module_sym& mod) noexcept
  {
    store_sym(mod, m_modules);
  }

}