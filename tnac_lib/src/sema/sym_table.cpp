#include "sema/sym_table.hpp"
#include "sema/symbol.hpp"

namespace tnac::semantics
{
  // Special members

  sym_table::~sym_table() noexcept = default;

  sym_table::sym_table() noexcept = default;

  // Public members

  scope& sym_table::add_scope(const ast::scope* node, const scope* parent) noexcept
  {
    auto&& insertedScope = m_scopes.emplace_back(std::make_unique<scope>(parent, node));
    return *insertedScope.get();
  }
}