#include "sema/sym/sym_func.hpp"

namespace tnac::semantics
{
  // Special members

  function::~function() noexcept = default;

  function::function(scope& owner, name_t name, param_list params, loc_t loc) noexcept :
    symbol{ kind::Function, name, owner, loc },
    m_params{ std::move(params) }
  {}


  // Public members

  function::size_type function::param_count() const noexcept
  {
    return m_params.size();
  }

  const function::param_list& function::params() const noexcept
  {
    return m_params;
  }
  function::param_list& function::params() noexcept
  {
    return FROM_CONST(params);
  }
}