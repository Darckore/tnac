#include "cfg/ir/ir_instructions.hpp"

namespace tnac::ir // virtual register
{
  // Special members

  vreg::~vreg() noexcept = default;

  vreg::vreg(from_id, id_type id, reg_scope scope) noexcept :
    node{ kind::Register },
    m_id{ id },
    m_scope{ scope }
  {}

  vreg::vreg(string_t name, reg_scope scope) noexcept :
    vreg{ from_id{}, name, scope }
  {}

  vreg::vreg(idx_type idx, reg_scope scope) noexcept :
    vreg{ from_id{}, idx, scope }
  {}


  // Public members

  bool vreg::is_named() const noexcept
  {
    return std::holds_alternative<string_t>(m_id);
  }

  string_t vreg::name() const noexcept
  {
    UTILS_ASSERT(is_named());
    return std::get<string_t>(m_id);
  }

  vreg::idx_type vreg::index() const noexcept
  {
    UTILS_ASSERT(!is_named());
    return std::get<idx_type>(m_id);
  }

  bool vreg::is_global() const noexcept
  {
    return m_scope == Global;
  }
}


namespace tnac::ir // param
{
  func_param::~func_param() noexcept = default;

  func_param::func_param(value_type val) noexcept :
    m_value{ val }
  {}

  func_param::value_type func_param::operator*() const noexcept
  {
    return m_value;
  }
}


namespace tnac::ir // operand
{
  // Special members

  operand::~operand() noexcept = default;


  // Public members

  bool operand::is_undef() const noexcept
  {
    if (!is_value())
      return false;
    return !get_value();
  }

  bool operand::is_value() const noexcept
  {
    return std::holds_alternative<eval::value>(m_value);
  }
  eval::value operand::get_value() const noexcept
  {
    UTILS_ASSERT(is_value());
    return std::get<eval::value>(m_value);
  }

  bool operand::is_register() const noexcept
  {
    return std::holds_alternative<vreg*>(m_value);
  }
  vreg& operand::get_reg() const noexcept
  {
    UTILS_ASSERT(is_register());
    return *std::get<vreg*>(m_value);
  }

  bool operand::is_param() const noexcept
  {
    return std::holds_alternative<func_param>(m_value);
  }
  func_param operand::get_param() const noexcept
  {
    UTILS_ASSERT(is_param());
    return std::get<func_param>(m_value);
  }

  bool operand::is_block() const noexcept
  {
    return std::holds_alternative<basic_block*>(m_value);
  }
  basic_block& operand::get_block() const noexcept
  {
    UTILS_ASSERT(is_block());
    return *std::get<basic_block*>(m_value);
  }

  bool operand::is_edge() const noexcept
  {
    return std::holds_alternative<edge*>(m_value);
  }
  edge& operand::get_edge() const noexcept
  {
    UTILS_ASSERT(is_edge());
    return *std::get<edge*>(m_value);
  }

  bool operand::is_index() const noexcept
  {
    return std::holds_alternative<idx_type>(m_value);
  }
  operand::idx_type operand::get_index() const noexcept
  {
    UTILS_ASSERT(is_index());
    return std::get<idx_type>(m_value);
  }
}


namespace tnac::ir // instruction
{
  // Special members

  instruction::~instruction() noexcept = default;

  instruction::instruction(basic_block& owner, op_code code, size_type count) noexcept :
    node{ kind::Instruction },
    m_block{ &owner },
    m_opCode{ code }
  {
    prealloc(count);
  }

  instruction::instruction(basic_block& owner, op_code code) noexcept :
    instruction{ owner, code, estimate_op_count(code) }
  {}


  const operand& instruction::operator[](size_type idx) const noexcept
  {
    UTILS_ASSERT(idx < operand_count());
    return m_operands[idx];
  }

  // Public members

  string_t instruction::opcode_str(op_code oc) noexcept
  {
    switch (oc)
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
    case CmpNE: return "cmpne"sv;
    case CmpG:  return "cmpg"sv;
    case CmpGE: return "cmpge"sv;

    case Abs:    return "abs"sv;
    case Plus:   return "aplus"sv;
    case Neg:    return "neg"sv;
    case BNeg:   return "bitneg"sv;
    case CmpNot: return "not"sv;
    case CmpIs:  return "is"sv;

    case Alloc:  return "alloc"sv;
    case Arr:    return "arr"sv;
    case Store:  return "store"sv;
    case Load:   return "load"sv;
    case Append: return "append"sv;

    case Select: return "sel"sv;
    case Call:   return "call"sv;
    case Jump:   return "jmp"sv;
    case Ret:    return "ret"sv;

    case Phi:    return "phi"sv;

    case Bool:   return "bool"sv;
    case Int:    return "int"sv;
    case Float:  return "float"sv;
    case Frac:   return "frac"sv;
    case Cplx:   return "cplx"sv;
    }

    UTILS_ASSERT(false);
    return {};
  }

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
    return opcode_str(m_opCode);
  }

  instruction::size_type instruction::operand_count() const noexcept
  {
    return m_operands.size();
  }


  // Private members

  instruction::size_type instruction::estimate_op_count(op_code code) noexcept
  {
    using eval::type_info;
    auto count = size_type{};
    switch (code)
    {
    case Add:    count = 3; break;
    case Sub:    count = 3; break;
    case Mul:    count = 3; break;
    case Div:    count = 3; break;
    case Mod:    count = 3; break;
    case Pow:    count = 3; break;
    case Root:   count = 3; break;
    case And:    count = 3; break;
    case Or:     count = 3; break;
    case Xor:    count = 3; break;
    case CmpE:   count = 3; break;
    case CmpL:   count = 3; break;
    case CmpLE:  count = 3; break;

    case Abs:    count = 2; break;
    case Plus:   count = 2; break;
    case Neg:    count = 2; break;
    case BNeg:   count = 2; break;
    case CmpNot: count = 2; break;
    case CmpIs:  count = 2; break;

    case Store:  count = 2; break;
    case Load:   count = 2; break;
    case Alloc:  count = 1; break;
    case Arr:    count = 2; break;
    case Append: count = 2; break;

    case Select: count = 4; break;
    case Call:   count = 2; break;
    case Jump:   count = 1; break;

    case Ret:    count = 1; break;

    case Phi:    count = 3; break; // 2 branches is the most common case (probably)

    case Bool:   count = type_info<eval::bool_type>::maxArgs + 1;     break;
    case Int:    count = type_info<eval::int_type>::maxArgs + 1;      break;
    case Float:  count = type_info<eval::float_type>::maxArgs + 1;    break;
    case Frac:   count = type_info<eval::fraction_type>::maxArgs + 1; break;
    case Cplx:   count = type_info<eval::complex_type>::maxArgs + 1;  break;
    }

    return count;
  }

  void instruction::prealloc(size_type size) noexcept
  {
    m_operands.reserve(size);
  }

}