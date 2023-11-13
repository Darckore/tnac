#include "sema/sym/sym_scope.hpp"

namespace tnac::semantics
{
  // Special members

  scope::~scope() noexcept = default;

  scope::scope(scope* encl) noexcept :
    m_enclosing{ encl }
  {
    if (m_enclosing)
      m_depth = m_enclosing->depth() + 1;
  }


  // Public members

  const scope* scope::enclosing() const noexcept
  {
    return m_enclosing;
  }
  scope* scope::enclosing() noexcept
  {
    return FROM_CONST(enclosing);
  }

  scope::depth_t scope::depth() const noexcept
  {
    return m_depth;
  }
}