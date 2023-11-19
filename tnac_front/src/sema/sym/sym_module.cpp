#include "sema/sym/sym_module.hpp"

namespace tnac::semantics // Module
{
  // Special members

  module_sym::~module_sym() noexcept = default;

  module_sym::module_sym(scope& owner, name_t name, loc_t loc) noexcept :
    function{ kind::Module, owner, name, {}, loc }
  {}

  // Protected members

  void module_sym::attach_params(param_list params) noexcept
  {
    function::attach_params(std::move(params));
  }

  void module_sym::override_location(loc_t loc) noexcept
  {
    symbol::override_location(loc);
  }
}


namespace tnac::semantics // Scope ref
{
  // Special members

  scope_ref::~scope_ref() noexcept = default;

  scope_ref::scope_ref(scope& owner, name_t name, scope& referenced, loc_t loc) noexcept :
    symbol{ kind::ScopeRef, name, owner, loc },
    m_referenced{ &referenced }
  {}


  // Public members

  const scope& scope_ref::referenced() const noexcept
  {
    return *m_referenced;
  }
  scope& scope_ref::referenced() noexcept
  {
    return FROM_CONST(referenced);
  }
}