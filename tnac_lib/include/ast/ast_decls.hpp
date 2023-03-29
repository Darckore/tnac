//
// Nodes for declarations
//

#pragma once
#include "ast/ast.hpp"
#include "ast/ast_expr.hpp"

namespace tnac::ast
{
  //
  // Base declarator
  //
  class decl : public node
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(decl);

    virtual ~decl() noexcept;

  protected:
    decl(kind k, id_expr* id, expr* def) noexcept;

  public:
    string_t name() const noexcept;

    const expr* definition() const noexcept;
    expr* definition() noexcept;

  private:
    id_expr* m_id{};
    expr* m_def{};
  };
}