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

      if (encl->has_sym())
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
  bool scope::is_module() const noexcept
  {
    return is(Module);
  }
  bool scope::is_function() const noexcept
  {
    return is(Function);
  }
  bool scope::is_internal() const noexcept
  {
    return is(Block);
  }

  bool scope::has_sym() const noexcept
  {
    return static_cast<bool>(sym());
  }

  const symbol* scope::sym() const noexcept
  {
    return m_sym;
  }
  symbol* scope::sym() noexcept
  {
    return FROM_CONST(sym);
  }

  const function* scope::to_func() const noexcept
  {
    return utils::try_cast<function>(sym());
  }
  function* scope::to_func() noexcept
  {
    return FROM_CONST(to_func);
  }

  const function& scope::func() const noexcept
  {
    auto fsym = to_func();
    UTILS_ASSERT(static_cast<bool>(fsym));
    return *fsym;
  }
  function& scope::func() noexcept
  {
    return FROM_CONST(func);
  }

  const module_sym* scope::to_module() const noexcept
  {
    return utils::try_cast<module_sym>(sym());
  }
  module_sym* scope::to_module() noexcept
  {
    return FROM_CONST(to_module);
  }

  const module_sym& scope::mod() const noexcept
  {
    auto msym = to_module();
    UTILS_ASSERT(static_cast<bool>(msym));
    return *msym;
  }
  module_sym& scope::mod() noexcept
  {
    return FROM_CONST(mod);
  }

  const scope_ref* scope::to_scope_ref() const noexcept
  {
    return utils::try_cast<scope_ref>(sym());
  }
  scope_ref* scope::to_scope_ref() noexcept
  {
    return FROM_CONST(to_scope_ref);
  }

  const scope_ref& scope::sc_ref() const noexcept
  {
    auto sr = to_scope_ref();
    UTILS_ASSERT(static_cast<bool>(sr));
    return *sr;
  }
  scope_ref& scope::sc_ref() noexcept
  {
    return FROM_CONST(sc_ref);
  }


  // Protected members

  void scope::attach_symbol(symbol& sym) noexcept
  {
    m_sym = &sym;
  }
}