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

  inline auto get_id(const expr& e) noexcept
  {
    return e.what();
  }

  //
  // Result expression
  // Corresponds to the _result keyword
  // Holds the last evaluated value
  //
  class result_expr final : public expr
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(result_expr);

    virtual ~result_expr() noexcept;

  protected:
    result_expr(const token& tok) noexcept;
  };


  //
  // Ret expression
  // Returns control to the caller
  //
  class ret_expr final : public expr
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(ret_expr);

    virtual ~ret_expr() noexcept;

  protected:
    ret_expr(expr& retVal, const token& kwPos) noexcept;

  public:
    //
    // Returns the value to propagate to the caller
    // 
    // const version
    //
    const expr& returned_value() const noexcept;

    //
    // Returns the value to propagate to the caller
    // 
    expr& returned_value() noexcept;

  private:
    expr* m_retVal{};
  };


  //
  // Literal expression
  // Corresponds to any value literals of supported types
  //
  class lit_expr final : public expr
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(lit_expr);

    virtual ~lit_expr() noexcept;

  protected:
    lit_expr(const token& tok) noexcept;
  };


  //
  // Id expression
  // Corresponds to references to any previously declared entities
  //
  class id_expr final : public expr
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
  class unary_expr final : public expr
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
  class assign_expr final : public binary_expr
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
  class paren_expr final : public expr
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


  //
  // Any invocation which has a name and a set of arguments
  //
  class invocation : public expr
  {
  public:
    using arg = expr;
    using pointer = arg*;
    using const_pointer = const arg*;

    using arg_list = std::vector<pointer>;

  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(invocation);

    virtual ~invocation() noexcept;

  protected:
    invocation(node_kind kind, const token& name, arg_list args) noexcept;

  public:
    //
    // Returns the type name token
    //
    const token& name() const noexcept;

    //
    // Returns the argument list
    // 
    // const version
    //
    const arg_list& args() const noexcept;

    //
    // Returns the argument list
    //
    arg_list& args() noexcept;

  private:
    arg_list m_args;
  };

  inline auto get_id(const invocation& e) noexcept
  {
    return e.what();
  }


  //
  // Typed expression
  // Represents a value of the given type initialised by an arg list
  //
  class typed_expr final : public invocation
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(typed_expr);

    virtual ~typed_expr() noexcept;

  protected:
    typed_expr(const token& typeName, arg_list args) noexcept;

  public:
    //
    // Returns the type name token
    //
    const token& type_name() const noexcept;
  };


  //
  // Call expression
  // Represents a call to a named callable entity with an arg list
  //
  class call_expr final : public expr
  {
  public:
    using arg_list = invocation::arg_list;

  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(call_expr);

    virtual ~call_expr() noexcept;

  protected:
    call_expr(expr& callable, arg_list args) noexcept;

  public:
    //
    // Returns the callee expr
    // 
    // const version
    //
    const expr& callable() const noexcept;

    //
    // Returns the callee expr
    //
    expr& callable() noexcept;

    //
    // Returns the argument list
    // 
    // const version
    //
    const arg_list& args() const noexcept;

    //
    // Returns the argument list
    //
    arg_list& args() noexcept;

  private:
    arg_list m_args;
    expr* m_callee{};
  };


  //
  // Pattern
  // Used in conditional expressions
  //
  class pattern final : public expr
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(pattern);

    virtual ~pattern() noexcept;

  protected:
    pattern(const token& op, expr* checkedExpr, scope& body) noexcept;

  public:
    //
    // Checks whether the pattern is a default one
    // Returns true is the checked expr is null
    //
    bool is_default() const noexcept;

    //
    // Returns the checked expression
    //
    // const version
    // 
    const expr& checked() const noexcept;

    //
    // Returns the checked expression
    //
    expr& checked() noexcept;

    //
    // Returns the body
    //
    // const version
    // 
    const scope& body() const noexcept;

    //
    // Returns the body
    //
    scope& body() noexcept;

    //
    // Checks whether the pattern operator is implicit
    //
    bool has_implicit_op() const noexcept;

  private:
    expr*  m_checked{};
    scope* m_body{};
  };


  //
  // Conditional expression
  // Represents a pattern collection a condition is checked against
  //
  class cond_expr final : public expr
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(cond_expr);

    virtual ~cond_expr() noexcept;

  protected:
    cond_expr(expr& condition, scope& body) noexcept;

  public:
    //
    // Returns the condition
    // 
    // const version
    //
    const expr& cond() const noexcept;

    //
    // Returns the condition
    // 
    expr& cond() noexcept;

    //
    // Returns the pattern collection
    // 
    // const version
    //
    const scope& patterns() const noexcept;

    //
    // Returns the pattern collection
    // 
    scope& patterns() noexcept;

  private:
    expr* m_cond{};
    scope* m_body{};
  };
}