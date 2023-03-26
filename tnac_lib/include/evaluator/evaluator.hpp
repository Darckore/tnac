//
// Expression evaluator
//

#pragma once
#include "ast/ast_visitor.hpp"
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
    void visit(ast::unary_expr* unary) noexcept;

    void visit(ast::lit_expr* lit) noexcept;

  private:
    //
    // Evaluates a literal and returns its value
    //
    eval::value eval_token(const token& tok) noexcept;

    //
    // Reads an integer from string
    //
    eval::value eval_int(string_t src, int base) noexcept;

    //
    // Reads a float from string
    //
    eval::value eval_float(string_t src) noexcept;

  private:
    eval::registry& m_registry;
  };
}