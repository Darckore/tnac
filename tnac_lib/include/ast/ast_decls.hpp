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
    decl(kind k, const token& id, expr& def) noexcept;

  public:
    string_t name() const noexcept;

    const expr* definition() const noexcept;
    expr* definition() noexcept;

    const token& pos() const noexcept;

  private:
    expr* m_def{};
    token m_id{};
  };


  //
  // Decl Expression
  //
  class decl_expr : public expr
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(decl_expr);

    virtual ~decl_expr() noexcept;

  protected:
    decl_expr(decl& d) noexcept;

  public:
    const decl& declarator() const noexcept;
    decl& declarator() noexcept;

  private:
    decl* m_decl{};
  };


  //
  // Variable decl
  //
  class var_decl : public decl
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(var_decl);

    virtual ~var_decl() noexcept;

  protected:
    var_decl(const token& var, expr& initialiser) noexcept;
  };
}