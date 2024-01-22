#include "cfg/ir/ir_basic_block.hpp"

namespace tnac::ir // edge
{
  // Special members

  edge::~edge() noexcept = default;

  edge::edge(basic_block& in, basic_block& out) noexcept :
    m_in{ &in },
    m_out{ &out }
  {}


  // Public members

  const basic_block& edge::incoming() const noexcept
  {
    return *m_in;
  }
  basic_block& edge::incoming() noexcept
  {
    return FROM_CONST(incoming);
  }

  const basic_block& edge::outgoing() const noexcept
  {
    return *m_out;
  }
  basic_block& edge::outgoing() noexcept
  {
    return FROM_CONST(outgoing);
  }
}


namespace tnac::ir // basic block
{
  // Special members

  basic_block::~basic_block() noexcept = default;

  basic_block::basic_block(string_t name, function& owner) noexcept :
    node{ kind::Block },
    m_owner{ &owner },
    m_name{ name }
  {}


  // Public members

  string_t basic_block::name() const noexcept
  {
    return m_name;
  }

  const function& basic_block::func() const noexcept
  {
    return *m_owner;
  }
  function& basic_block::func() noexcept
  {
    return FROM_CONST(func);
  }
}


namespace tnac::ir::detail // block container
{
  // Special members

  block_container::~block_container() noexcept = default;

  block_container::block_container(reference value) noexcept :
    m_value{ &value }
  {}


  // Public members

  const basic_block* block_container::find(string_t name) const noexcept
  {
    auto found = m_value->find(name);
    return found != m_value->end() ? &found->second : nullptr;
  }
  basic_block* block_container::find(string_t name) noexcept
  {
    return FROM_CONST(find, name);
  }

  void block_container::remove(string_t name) noexcept
  {
    m_value->erase(name);
  }

  void block_container::remove(block_iterator it) noexcept
  {
    m_value->erase(it.get());
  }

  basic_block& block_container::add(string_t name, function& owner) noexcept
  {
    auto newItem = m_value->try_emplace(name, name, owner);
    UTILS_ASSERT(newItem.second);
    return newItem.first->second;
  }
}