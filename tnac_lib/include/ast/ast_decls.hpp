//
// Nodes for declarations
//

#pragma once
#include "ast/ast.hpp"
#include "ast/ast_expr.hpp"

namespace tnac::semantics
{
  class symbol;
}

namespace tnac::ast
{
  //
  // Base declarator for all entities that can be defined
  //
  class decl : public node
  {
  private:
    friend class builder;
    friend class semantics::symbol;

  public:
    CLASS_SPECIALS_NONE(decl);

    virtual ~decl() noexcept;

  protected:
    decl(kind k, const token& id, expr& def) noexcept;

    //
    // Attaches a symbol to this declarator. Called from the corresponding
    // symbol object on its creation
    //
    void attach_symbol(semantics::symbol& sym) noexcept;

  public:
    //
    // Returns the entity name
    //
    string_t name() const noexcept;

    //
    // Returns the definition expression which is present at the right side
    // of the assignment operator
    // 
    // const version
    //
    const expr& definition() const noexcept;

    //
    // Returns the definition expression which is present at the right side
    // of the assignment operator
    // 
    expr& definition() noexcept;

    //
    // Returns the first token associated with this declarator
    //
    const token& pos() const noexcept;

    //
    // Returns the attached symbol
    // 
    // const version
    //
    const semantics::symbol& symbol() const noexcept;

    //
    // Returns the attached symbol
    // 
    semantics::symbol& symbol() noexcept;

  private:
    expr* m_def{};
    semantics::symbol* m_symbol{};
    token m_id{};
  };


  //
  // Decl expression. Holds the declarator corresponding to the declared entity
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
    //
    // Returns the declarator
    // 
    // const version
    //
    const decl& declarator() const noexcept;

    //
    // Returns the declarator
    // 
    decl& declarator() noexcept;

  private:
    decl* m_decl{};
  };


  //
  // Variable declarator
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