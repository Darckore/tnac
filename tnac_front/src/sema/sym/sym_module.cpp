#include "sema/sym/sym_module.hpp"

namespace tnac::semantics
{
  // Special members

  module_ref::~module_ref() noexcept = default;

  module_ref::module_ref(scope& owner, name_t name, loc_t loc) noexcept :
    symbol{ kind::Module, name, owner, loc }
  {}


  // Public members

  module_ref::size_type module_ref::param_count() const noexcept
  {
    return m_params.size();
  }

  const module_ref::param_list& module_ref::params() const noexcept
  {
    return m_params;
  }
  module_ref::param_list& module_ref::params() noexcept
  {
    return FROM_CONST(params);
  }
}