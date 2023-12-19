#include "cfg/ir/ir_function.hpp"

namespace tnac::ir
{
  // Special members

  function::~function() noexcept = default;

  function::function(name_t name, size_type paramCount, block_list blocks) noexcept :
    function{ name, paramCount, {}, blocks }
  {}

  function::function(name_t name, size_type paramCount, function& owner, block_list blocks) noexcept :
    function{ name, paramCount, &owner, blocks }
  {
    owner.add_child(*this);
  }

  function::function(name_t name, size_type paramCount, function* owner, block_list blocks) noexcept :
    node{ kind::Function },
    m_name{ name },
    m_owner{ owner },
    m_blocks{ blocks },
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

  const function::child_list& function::children() const noexcept
  {
    return m_children;
  }
  function::child_list& function::children() noexcept
  {
    return FROM_CONST(children);
  }


  // Private members

  void function::add_child(function& child) noexcept
  {
    m_children.push_back(&child);
  }
}