#include "sema/sym/sym_scope.hpp"

namespace tnac::semantics
{
  // Special members

  scope::~scope() noexcept = default;

  scope::scope(scope* encl) noexcept :
    m_enclosing{ encl }
  {}


  // Public members

  const scope* scope::enclosing() const noexcept
  {
    return m_enclosing;
  }
  scope* scope::enclosing() noexcept
  {
    return FROM_CONST(enclosing);
  }
}