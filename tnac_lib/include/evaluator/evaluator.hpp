//
// Expression evaluator
//

#pragma once
#include "ast/ast_visitor.hpp"

namespace tnac
{
  //
  // Evaluator for expressions
  //
  class evaluator : public ast::bottom_up_visitor<evaluator>
  {
  public:
    //CLASS_SPECIALS_NODEFAULT(evaluator);
    CLASS_SPECIALS_ALL(evaluator);

  public:
    void visit(ast::lit_expr* lit) noexcept;
  };
}