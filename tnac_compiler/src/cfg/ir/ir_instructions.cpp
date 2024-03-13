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
  {
    prealloc();
  }


  // Public members

  op_code instruction::opcode() const noexcept
  {
    return m_opCode;
  }

  const basic_block& instruction::owner_block() const noexcept
  {
    return *m_block;
  }
  basic_block& instruction::owner_block() noexcept
  {
    return FROM_CONST(owner_block);
  }

  instruction& instruction::add(operand op) noexcept
  {
    m_operands.push_back(std::move(op));
    return *this;
  }


  // Private members

  void instruction::prealloc() noexcept
  {
    auto count = size_type{};
    switch (m_opCode)
    {
    case Add:   break;
    case Sub:   break;
    case Mul:   break;
    case Div:   break;
    case Mod:   break;
    case Pow:   break;
    case Root:  break;
    case And:   break;
    case Or:    break;
    case Xor:   break;
    case CmpE:  break;
    case CmpL:  break;
    case CmpLE: break;

    case Abs:   break;

    case Store: break;
    case Load:  break;
    case Call:  break;
    case Jump:  break;
    case Ret:   count = 1; break;
    }

    m_operands.reserve(count);
  }

}