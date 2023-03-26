#include "evaluator/evaluator.hpp"
#include "evaluator/value.hpp"

namespace tnac
{
  // Special members



  // Public members

  void evaluator::visit(ast::lit_expr* lit) noexcept
  {
    utils::unused(lit);
  }

}