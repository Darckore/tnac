#include "sema/sym_table.hpp"
#include "sema/symbol.hpp"
#include "parser/ast/ast_decls.hpp"

namespace tnac::semantics
{
  // Special members

  sym_table::~sym_table() noexcept = default;

  sym_table::sym_table() noexcept = default;

  // Public members

  const scope& sym_table::add_scope(const ast::scope* node, scope_ptr parent) noexcept
  {
    auto&& insertedScope = m_scopes.emplace_back(std::make_unique<scope>(parent, node));
    return *insertedScope.get();
  }

  variable& sym_table::add_variable(ast::decl& decl, scope_ptr parent) noexcept
  {
    return *make_symbol<variable>(decl.name(), parent, decl);
  }

  parameter& sym_table::add_parameter(ast::decl& decl, scope_ptr parent) noexcept
  {
    return *make_symbol<parameter>(decl.name(), parent, decl);
  }

  function& sym_table::add_function(ast::decl& decl, scope_ptr parent) noexcept
  {
    return *make_symbol<function>(decl.name(), parent, decl);
  }

  sym_table::sym_ptr sym_table::lookup(string_t name, scope_ptr parent) noexcept
  {
    auto scopes = lookup(name);
    return lookup(scopes, parent, false);
  }

  sym_table::sym_ptr sym_table::scoped_lookup(string_t name, scope_ptr parent) noexcept
  {
    auto scopes = lookup(name);
    return lookup(scopes, parent, true);
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

  sym_table::scope_map* sym_table::lookup(string_t name) noexcept
  {
    auto nameIt = m_names.find(name);
    if (nameIt == m_names.end())
      return {};

    return &nameIt->second;
  }

  sym_table::scope_map& sym_table::make_name(string_t name) noexcept
  {
    return m_names.emplace(name, scope_map{}).first->second;
  }

  sym_table::sym_ptr& sym_table::make_symbol(scope_map& scopes, scope_ptr parent) noexcept
  {
    return scopes.emplace(parent, sym_ptr{}).first->second;
  }

}