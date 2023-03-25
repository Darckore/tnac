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
    expr(kind k) noexcept;
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
    lit_expr(const token& tok) noexcept;

  private:
    token m_value{};
  };


  //
  // Unary expr
  //
  class unary_expr : public expr
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(unary_expr);

    virtual ~unary_expr() noexcept;

  protected:
    unary_expr(expr& e, const token& op) noexcept;

  private:
    expr* m_expr{};
    token m_op{};
  };
}