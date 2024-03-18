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


  const operand& instruction::operator[](size_type idx) const noexcept
  {
    UTILS_ASSERT(idx < operand_count());
    return m_operands[idx];
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

  string_t instruction::opcode_str() const noexcept
  {
    switch (m_opCode)
    {
    case Add:   return "add"sv;
    case Sub:   return "sub"sv;
    case Mul:   return "mul"sv;
    case Div:   return "div"sv;
    case Mod:   return "mod"sv;
    case Pow:   return "pow"sv;
    case Root:  return "root"sv;
    case And:   return "and"sv;
    case Or:    return "or"sv;
    case Xor:   return "xor"sv;
    case CmpE:  return "cmpe"sv;
    case CmpL:  return "cmpl"sv;
    case CmpLE: return "cmple"sv;

    case Abs:   return "abs"sv;

    case Store: return "store"sv;
    case Load:  return "load"sv;
    case Call:  return "call"sv;
    case Jump:  return "jmp"sv;
    case Ret:   return "ret"sv;
    }

    UTILS_ASSERT(false);
    return {};
  }


  // Private members

  void instruction::prealloc() noexcept
  {
    auto count = size_type{};
    switch (m_opCode)
    {
    case Add:   count = 3; break;
    case Sub:   count = 3; break;
    case Mul:   count = 3; break;
    case Div:   count = 3; break;
    case Mod:   count = 3; break;
    case Pow:   count = 3; break;
    case Root:  count = 3; break;
    case And:   count = 3; break;
    case Or:    count = 3; break;
    case Xor:   count = 3; break;
    case CmpE:  count = 3; break;
    case CmpL:  count = 3; break;
    case CmpLE: count = 3; break;

    case Abs:   count = 2; break;

    case Store: count = 2; break;
    case Load:  count = 2; break;
    case Alloc: count = 2; break;

    case Call:  count = 2; break;
    case Jump:  count = 1; break;

    case Ret:   count = 1; break;
    }

    m_operands.reserve(count);
  }

  instruction::size_type instruction::operand_count() const noexcept
  {
    return m_operands.size();
  }

}