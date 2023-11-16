#include "sema/sym/sym_module.hpp"

namespace tnac::semantics // Module
{
  // Special members

  module_sym::~module_sym() noexcept = default;

  module_sym::module_sym(scope& owner, name_t name, loc_t loc) noexcept :
    symbol{ kind::Module, name, owner, loc }
  {}


  // Public members

  module_sym::size_type module_sym::param_count() const noexcept
  {
    return m_params.size();
  }

  const module_sym::param_list& module_sym::params() const noexcept
  {
    return m_params;
  }
  module_sym::param_list& module_sym::params() noexcept
  {
    return FROM_CONST(params);
  }


  // Protected members

  void module_sym::attach_params(param_list params) noexcept
  {
    UTILS_ASSERT(m_params.empty());
    m_params = std::move(params);
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
}