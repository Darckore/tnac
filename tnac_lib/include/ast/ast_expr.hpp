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

  public:
    const token& value() const noexcept;

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

  public:
    const token& op() const noexcept;

    const expr& operand() const noexcept;
    expr& operand() noexcept;

  private:
    expr* m_expr{};
    token m_op{};
  };


  //
  // Binary expr
  //
  class binary_expr : public expr
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(binary_expr);

    virtual ~binary_expr() noexcept;

  protected:
    binary_expr(expr& left, expr& right, const token& op) noexcept;

  public:
    const token& op() const noexcept;

    const expr& left() const noexcept;
    expr& left() noexcept;

    const expr& right() const noexcept;
    expr& right() noexcept;

  private:
    expr* m_left{};
    expr* m_right{};
    token m_op{};
  };


  //
  // Paren expr
  //
  class paren_expr : public expr
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(paren_expr);

    virtual ~paren_expr() noexcept;

  protected:
    paren_expr(expr& e) noexcept;

  public:
    const expr& internal_expr() const noexcept;
    expr& internal_expr() noexcept;

  private:
    expr* m_expr{};
  };
}