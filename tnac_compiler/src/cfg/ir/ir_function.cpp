#include "cfg/ir/ir_function.hpp"

namespace tnac::ir
{
  // Special members

  function::~function() noexcept = default;

  function::function(name_t name, entity_id id, size_type paramCount, block_list blocks) noexcept :
    function{ name, id, paramCount, {}, std::move(blocks) }
  {}

  function::function(name_t name, entity_id id, size_type paramCount, function& owner, block_list blocks) noexcept :
    function{ name, id, paramCount, &owner, std::move(blocks) }
  {
    owner.add_child(*this);
  }

  function::function(name_t name, entity_id id, size_type paramCount, function* owner, block_list blocks) noexcept :
    node{ kind::Function },
    m_name{ name },
    m_owner{ owner },
    m_blocks{ std::move(blocks) },
    m_id{ id },
    m_paramCount{ paramCount }
  {}


  // Public members

  function::name_t function::name() const noexcept
  {
    return m_name;
  }

  entity_id function::id() const noexcept
  {
    return m_id;
  }

  function::size_type function::param_count() const noexcept
  {
    return m_paramCount;
  }

  const basic_block& function::entry() const noexcept
  {
    UTILS_ASSERT(m_entry);
    return *m_entry;
  }
  basic_block& function::entry() noexcept
  {
    return FROM_CONST(entry);
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

  const function::block_list& function::blocks() const noexcept
  {
    return m_blocks;
  }
  function::block_list& function::blocks() noexcept
  {
    return FROM_CONST(blocks);
  }

  basic_block& function::create_block(string_t name) noexcept
  {
    auto&& block = m_blocks.add(name, *this);
    if (!m_entry)
      m_entry = &block;

    return block;
  }


  // Private members

  void function::add_child(function& child) noexcept
  {
    m_children.push_back(&child);
  }
}