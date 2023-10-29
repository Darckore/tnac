#include "ir/ir.hpp"

namespace tnac::ir // operation
{
  // Special members

  operation::~operation() noexcept = default;

  operation::operation(data_t data) noexcept :
    m_data{ data }
  {}


  // Public members

  op_code operation::code() const noexcept
  {
    return std::bit_cast<op_code>(m_data.front());
  }
}