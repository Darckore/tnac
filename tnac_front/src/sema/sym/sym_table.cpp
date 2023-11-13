#include "sema/sym/sym_table.hpp"
#include "sema/sym/sym_base.hpp"

namespace tnac::semantics
{
  // Special members

  sym_table::~sym_table() noexcept = default;

  sym_table::sym_table() noexcept = default;

  // Public members

  const scope& sym_table::add_scope(scope_ptr parent) noexcept
  {
    auto&& insertedScope = m_scopes.emplace_back(std::make_unique<scope>(parent));
    return *insertedScope.get();
  }

  variable& sym_table::add_variable(name_t name, scope_ptr parent) noexcept
  {
    auto&& newVar = *make_symbol<variable>(name, parent);
    store_variable(newVar);
    return newVar;
  }

  parameter& sym_table::add_parameter(name_t name, scope_ptr parent) noexcept
  {
    return *make_symbol<parameter>(name, parent);
  }

  function& sym_table::add_function(name_t name, scope_ptr parent, function::param_list params) noexcept
  {
    auto&& newFunc = *make_symbol<function>(name, parent, std::move(params));
    store_function(newFunc);
    return newFunc;
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

  // Private members

  sym_table::sym_ptr sym_table::lookup(scope_map* scopes, scope_ptr parent, bool current) noexcept
  {
    sym_ptr res{};
    if (!scopes)
      return res;

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

      parent = parent->m_enclosing;
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

}