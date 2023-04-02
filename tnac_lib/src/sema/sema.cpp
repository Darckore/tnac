#include "sema/sema.hpp"
#include "sema/symbol.hpp"
#include "ast/ast_nodes.hpp"

namespace tnac
{
  // Special members

  sema::~sema() noexcept = default;

  sema::sema() noexcept = default;

  // Public members

  void sema::open_scope(const ast::scope& scopeNode) noexcept
  {
    m_curScope = &m_symTab.add_scope(&scopeNode, m_curScope);
  }

  void sema::close_scope() noexcept
  {
    if(m_curScope)
      m_curScope = m_curScope->m_enclosing;
  }

  sema::sym_ptr sema::find(string_t name) const noexcept
  {
    utils::unused(name);
    return {};
  }

  void sema::visit_decl(ast::decl& decl) noexcept
  {
    utils::unused(decl);
  }
}