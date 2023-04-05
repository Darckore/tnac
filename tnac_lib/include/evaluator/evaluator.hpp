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
  //
  class evaluator : public ast::bottom_up_visitor<evaluator>
  {
  public:
    CLASS_SPECIALS_NONE(evaluator);

    explicit evaluator(eval::registry& registry) noexcept;

  public: // expressions
    void visit(ast::binary_expr* binary) noexcept;

    void visit(ast::unary_expr* unary) noexcept;

    void visit(ast::paren_expr* paren) noexcept;

    void visit(ast::lit_expr* lit) noexcept;

  public: // decls
    void visit(ast::decl_expr* expr) noexcept;

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