#include "sema/sym/sym_func.hpp"

namespace tnac::semantics
{
  // Special members

  function::~function() noexcept = default;

  function::function(scope& owner, name_t name, param_list params, scope& owned, loc_t loc) noexcept :
    function{ kind::Function, owner, name, std::move(params), owned, loc }
  {}

  function::function(kind k, scope& owner, name_t name, param_list params, scope& owned, loc_t loc) noexcept :
    symbol{ k, name, owner, loc },
    m_params{ std::move(params) },
    m_ownedScope{ &owned }
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

  const scope& function::own_scope() const noexcept
  {
    return *m_ownedScope;
  }
  scope& function::own_scope() noexcept
  {
    return FROM_CONST(own_scope);
  }


  // Protected members

  void function::attach_params(param_list params) noexcept
  {
    UTILS_ASSERT(m_params.empty());
    m_params = std::move(params);
  }

}