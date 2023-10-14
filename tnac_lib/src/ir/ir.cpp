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
}