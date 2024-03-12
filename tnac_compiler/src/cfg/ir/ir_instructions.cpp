#include "cfg/ir/ir_instructions.hpp"

namespace tnac::ir // operand
{
  // Special members

  operand::~operand() noexcept = default;


  // Public members

  bool operand::is_value() const noexcept
  {
    return std::holds_alternative<eval::value>(m_value);
  }
  eval::value operand::get_value() const noexcept
  {
    UTILS_ASSERT(is_value());
    return std::get<eval::value>(m_value);
  }
}


namespace tnac::ir // instruction
{
  // Special members

  instruction::~instruction() noexcept = default;

  instruction::instruction(basic_block& owner, op_code code) noexcept :
    node{ kind::Instruction },
    m_block{ &owner },
    m_opCode{ code }
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