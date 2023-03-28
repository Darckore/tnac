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

  public:
    void visit(ast::binary_expr* binary) noexcept;

    void visit(ast::unary_expr* unary) noexcept;

    void visit(ast::paren_expr* paren) noexcept;

    void visit(ast::lit_expr* lit) noexcept;

  private:
    //
    // Evaluates a literal and returns its value
    //
    eval::value eval_token(const token& tok) noexcept;

  private:
    eval::value_visitor m_visitor;
  };
}