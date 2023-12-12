#include "cfg/ir/ir_base.hpp"

namespace tnac::ir // instruction
{
  // Special members

  instruction::~instruction() noexcept = default;

  instruction::instruction(code c) noexcept :
    m_code{ c }
  {}


  // Public members

  instruction::code instruction::what() const noexcept
  {
    return m_code;
  }

}