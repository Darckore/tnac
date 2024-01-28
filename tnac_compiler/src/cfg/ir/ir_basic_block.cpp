#include "cfg/ir/ir_basic_block.hpp"

namespace tnac::ir // edge
{
  // Special members

  edge::~edge() noexcept = default;

  edge::edge(basic_block& in, basic_block& out) noexcept :
    node{ kind::Edge },
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