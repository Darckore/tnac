#include "sema/sym/sym_scope.hpp"
#include "sema/sym/symbols.hpp"

namespace tnac::semantics
{
  // Special members

  scope::~scope() noexcept = default;

  scope::scope(scope* encl, scope_kind kind) noexcept :
    m_enclosing{ encl },
    m_scopeKind{ kind }
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

  const scope* scope::encl_skip_internal() const noexcept
  {
    auto encl = enclosing();
    while(encl)
    {
      if (encl->is_function() || encl->is_top_level())
        break;

      encl = encl->enclosing();
    }
    return encl;
  }
  scope* scope::encl_skip_internal() noexcept
  {
    return FROM_CONST(encl_skip_internal);
  }

  scope::depth_t scope::depth() const noexcept
  {
    return m_depth;
  }

  scope_kind scope::kind() const noexcept
  {
    return m_scopeKind;
  }

  bool scope::is(scope_kind k) const noexcept
  {
    return kind() == k;
  }

  bool scope::is_top_level() const noexcept
  {
    return is_any(Global, Module);
  }
  bool scope::is_function() const noexcept
  {
    return is(Function);
  }
  bool scope::is_internal() const noexcept
  {
    return is(Block);
  }
}