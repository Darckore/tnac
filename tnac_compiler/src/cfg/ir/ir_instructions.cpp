#include "cfg/ir//ir_instructions.hpp"

namespace tnac::ir // instruction
{
  // Special members

  instruction::~instruction() noexcept = default;

  instruction::instruction(basic_block& owner, kind k) noexcept :
    node{ k },
    m_block{ &owner }
  {}


  // Public members

  const basic_block& instruction::owner_block() const noexcept
  {
    return *m_block;
  }
  basic_block& instruction::owner_block() noexcept
  {
    return FROM_CONST(owner_block);
  }

}