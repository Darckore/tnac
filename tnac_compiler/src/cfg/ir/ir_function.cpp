#include "cfg/ir/ir_function.hpp"

namespace tnac::ir
{
  // Special members

  function::~function() noexcept = default;

  function::function(name_t name) noexcept :
    function{ name, nullptr }
  {}

  function::function(name_t name, function& owner) noexcept :
    function{ name, &owner }
  {}

  function::function(name_t name, function* owner) noexcept :
    node{ kind::Function },
    m_name{ name },
    m_owner{ owner }
  {}


  // Public members

  function::name_t function::name() const noexcept
  {
    return m_name;
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