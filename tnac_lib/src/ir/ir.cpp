#include "ir/ir.hpp"

namespace tnac::ir
{
  // Special members

  operation::~operation() noexcept = default;

  operation::operation(op_code opCode, size_type prealloc) noexcept
  {
    m_data.reserve(prealloc + sizeof(opCode));
    m_data.emplace_back(std::bit_cast<byte_t>(opCode));
  }


  // Public members

  op_code operation::code() const noexcept
  {
    return std::bit_cast<op_code>(m_data.front());
  }

  void operation::add_value(eval::int_type val) noexcept
  {
    add_type_id(eval::type_id::Int);
    add_operand(val);
  }

  void operation::add_value(eval::float_type val) noexcept
  {
    add_type_id(eval::type_id::Float);
    add_operand(val);
  }

  void operation::add_value(eval::complex_type val) noexcept
  {
    add_type_id(eval::type_id::Complex);
    add_operand(val.real());
    add_operand(val.imag());
  }


  // Private members

  void operation::add_type_id(eval::type_id ti) noexcept
  {
    m_data.emplace_back(std::bit_cast<byte_t>(ti));
  }

}