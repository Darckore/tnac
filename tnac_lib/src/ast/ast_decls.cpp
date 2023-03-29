#include "ast/ast_decls.hpp"

namespace tnac::ast
{
  // Base declarator

  decl::~decl() noexcept = default;

  decl::decl(kind k, id_expr* id, expr* def) noexcept :
    node{ k },
    m_id{ id },
    m_def{ def }
  {
    assume_ancestry(m_id);
    assume_ancestry(m_def);
  }

  string_t decl::name() const noexcept
  {
    return m_id->name();
  }

  const expr* decl::definition() const noexcept
  {
    return m_def;
  }
  expr* decl::definition() noexcept
  {
    return FROM_CONST(definition);
  }
}