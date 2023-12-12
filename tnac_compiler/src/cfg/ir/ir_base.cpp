#include "cfg/ir/ir_base.hpp"

namespace tnac::ir // instruction
{
  // Special members

  instruction::~instruction() noexcept = default;

  instruction::instruction(basic_block& owner, code c) noexcept :
    m_block{ &owner },
    m_code{ c }
  {}


  // Public members

  instruction::code instruction::what() const noexcept
  {
    return m_code;
  }

  const basic_block& instruction::owner_block() const noexcept
  {
    return *m_block;
  }
  basic_block& instruction::owner_block() noexcept
  {
    return FROM_CONST(owner_block);
  }

}