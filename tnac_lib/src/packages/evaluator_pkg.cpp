#include "packages/evaluator_pkg.hpp"

namespace tnac::packages
{
  // Special members

  evaluator::~evaluator() noexcept = default;

  evaluator::evaluator(size_type stackSize) noexcept :
    m_callStack{ stackSize },
    m_evaluator{ m_registry, m_callStack }
  {}


  // Public members

  void evaluator::operator()(ast::node* root) noexcept
  {
    m_evaluator(root);
  }

  eval::value evaluator::last_result() const noexcept
  {
    return m_registry.evaluation_result();
  }
}