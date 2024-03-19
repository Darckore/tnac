#include "cfg/ir/ir_basic_block.hpp"

namespace tnac::ir // edge
{
  // Special members

  edge::~edge() noexcept = default;

  edge::edge(basic_block& in, basic_block& out, operand val) noexcept :
    node{ kind::Edge },
    m_in{ &in },
    m_out{ &out },
    m_value{ val }
  {
    m_in->add_out(this);
    m_out->add_pred(this);
  }


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

  operand edge::value() const noexcept
  {
    return m_value;
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

  basic_block& basic_block::add_instruction(instruction& in) noexcept
  {
    m_last = in.to_iterator();
    if (!m_first)
      m_first = m_last;

    return *this;
  }

  basic_block& basic_block::add_instruction_front(instruction& in) noexcept
  {
    m_first = in.to_iterator();
    if (!m_last)
      m_last = m_first;

    return *this;
  }

  basic_block::instruction_iter basic_block::begin() noexcept
  {
    return m_first;
  }
  basic_block::const_instruction_iter basic_block::begin() const noexcept
  {
    return m_first;
  }
  basic_block::instruction_iter basic_block::end() noexcept
  {
    return m_last ? std::next(m_last) : m_last;
  }
  basic_block::const_instruction_iter basic_block::end() const noexcept
  {
    return m_last ? std::next(m_last) : m_last;
  }


  // Protected members

  void basic_block::add_pred(edge* e) noexcept
  {
    m_in.push_back(e);
  }
  void basic_block::add_out(edge* e) noexcept
  {
    m_out.push_back(e);
  }
}