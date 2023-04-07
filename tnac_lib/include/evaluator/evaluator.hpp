//
// Expression evaluator
//

#pragma once
#include "ast/ast_visitor.hpp"
#include "evaluator/value_visitor.hpp"
#include "evaluator/value_registry.hpp"

namespace tnac
{
  //
  // Evaluator for expressions
  // Visits the provided ast node bottom-up (children first)
  //
  class evaluator : public ast::bottom_up_visitor<evaluator>
  {
  public:
    CLASS_SPECIALS_NONE(evaluator);

    explicit evaluator(eval::registry& registry) noexcept;

  public: // expressions
    //
    // Visits an assignment expression
    //
    void visit(ast::assign_expr* assign) noexcept;

    //
    // Visits a binary expression
    //
    void visit(ast::binary_expr* binary) noexcept;

    //
    // Visits a unary expression
    //
    void visit(ast::unary_expr* unary) noexcept;

    //
    // Visits a parenthesised expression
    //
    void visit(ast::paren_expr* paren) noexcept;

    //
    // Visits a literal expression
    //
    void visit(ast::lit_expr* lit) noexcept;

    //
    // Visits a variable reference expression
    //
    void visit(ast::id_expr* id) noexcept;

  public: // decls
    //
    // Visits a declaration expression
    //
    void visit(ast::decl_expr* expr) noexcept;

    //
    // Visits a variable declarator
    //
    void visit(ast::var_decl* decl) noexcept;

  private:
    //
    // Evaluates a literal and returns its value
    //
    eval::value eval_token(const token& tok) noexcept;

    //
    // Evaluates an assign expression and variable declarations 
    //
    void eval_assign(semantics::symbol& sym, eval::value rhs) noexcept;

  private:
    eval::value_visitor m_visitor;
  };
}