#include "cfg/ir/ir_function.hpp"

namespace tnac::ir
{
  // Special members

  function::~function() noexcept = default;

  function::function(name_t name, size_type paramCount) noexcept :
    function{ name, paramCount, nullptr }
  {}

  function::function(name_t name, size_type paramCount, function& owner) noexcept :
    function{ name, paramCount, &owner }
  {}

  function::function(name_t name, size_type paramCount, function* owner) noexcept :
    node{ kind::Function },
    m_name{ name },
    m_owner{ owner },
    m_paramCount{ paramCount }
  {}


  // Public members

  function::name_t function::name() const noexcept
  {
    return m_name;
  }

  function::size_type function::param_count() const noexcept
  {
    return m_paramCount;
  }

  const function* function::owner_func() const noexcept
  {
    return m_owner;
  }
  function* function::owner_func() noexcept
  {
    return FROM_CONST(owner_func);
  }
}