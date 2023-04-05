//
// Expression nodes
//

#pragma once
#include "ast/ast.hpp"
#include "evaluator/value.hpp"

namespace tnac::semantics
{
  class symbol;
}

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

    eval::value value() const noexcept;
    void eval_result(eval::value val) noexcept;
    const token& pos() const noexcept;

  protected:
    expr(kind k, const token& tok) noexcept;

  private:
    eval::value m_val;
    token m_pos;
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
    eval::value m_val;
  };


  //
  // Id expr
  //
  class id_expr : public expr
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(id_expr);

    virtual ~id_expr() noexcept;

  protected:
    id_expr(const token& tok, semantics::symbol& sym) noexcept;

  public:
    const semantics::symbol& symbol() const noexcept;
    semantics::symbol& symbol() noexcept;

    string_t name() const noexcept;

  private:
    semantics::symbol* m_sym{};
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
    binary_expr(kind k, expr& left, expr& right, const token& op) noexcept;
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
  // Assign expression
  //
  class assign_expr : public binary_expr
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(assign_expr);

    virtual ~assign_expr() noexcept;

  protected:
    assign_expr(expr& assignee, expr& assigned, const token& op) noexcept;
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
    paren_expr(expr& e, const token& op) noexcept;

  public:
    const expr& internal_expr() const noexcept;
    expr& internal_expr() noexcept;

  private:
    expr* m_expr{};
  };

}