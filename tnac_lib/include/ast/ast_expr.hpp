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
  // Base expression
  //
  class expr : public node
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(expr);

    virtual ~expr() noexcept;

  public:
    //
    // Returns the value associated with this expression
    //
    eval::value value() const noexcept;

    //
    // Assigns a value to this expression
    // The evaluator sets this
    //
    void eval_result(eval::value val) noexcept;

    //
    // Returns the first token associated with this expression
    //
    const token& pos() const noexcept;

  protected:
    expr(kind k, const token& tok) noexcept;

  private:
    eval::value m_val;
    token m_pos;
  };


  //
  // Literal expression
  // Corresponds to any value literals of supported types
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
  // Id expression
  // Corresponds to references to any previously declared entities
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

    //
    // Returns the referenced entity name
    //
    string_t name() const noexcept;

  private:
    semantics::symbol* m_sym{};
  };


  //
  // Unary expression
  // Provides info on the corresponding unary operator and its operand expression
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
    //
    // Returns the operator token
    //
    const token& op() const noexcept;

    //
    // Returns the operand
    // 
    // const version
    //
    const expr& operand() const noexcept;

    //
    // Returns the operand
    // 
    expr& operand() noexcept;

  private:
    expr* m_expr{};
  };


  //
  // Binary expression
  // Provides info on the corresponding binary operator and both its operands
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
    //
    // Returns the operator token
    //
    const token& op() const noexcept;

    //
    // Returns the left operand
    // 
    // const version
    //
    const expr& left() const noexcept;

    //
    // Returns the left operand
    //
    expr& left() noexcept;

    //
    // Returns the right operand
    // 
    // const version
    //
    const expr& right() const noexcept;

    //
    // Returns the right operand
    // 
    expr& right() noexcept;

  private:
    expr* m_left{};
    expr* m_right{};
    token m_op{};
  };


  //
  // Assign expression
  // Provides info on an assignment
  // 
  // Since it's a binary_expr, they share the interface
  // left returns the assigned-to expr
  // right returns the assigned-from expr
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
  // Paren expression
  // Represents any expression enclosed in parentheses
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
    //
    // Returns the expression in parentheses
    // 
    // const version
    //
    const expr& internal_expr() const noexcept;

    //
    // Returns the expression in parentheses
    // 
    expr& internal_expr() noexcept;

  private:
    expr* m_expr{};
  };

}