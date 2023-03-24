//
// Expression nodes
//

#pragma once
#include "ast/ast.hpp"

namespace tnac::ast
{
  //
  // Base expression class
  //
  class expr : public node
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(expr);

    virtual ~expr() noexcept;

  protected:
    expr(node* parent, kind k) noexcept;
  };


  //
  // Literal expr
  //
  class lit_expr : public expr
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(lit_expr);

    virtual ~lit_expr() noexcept;

  protected:
    lit_expr(node* parent, const token& tok) noexcept;

  private:
    token m_value{};
  };
}