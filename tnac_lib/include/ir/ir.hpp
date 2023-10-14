//
// IR
//

#pragma once
#include "ir/bytecode.hpp"

namespace tnac::ir
{
  //
  // Intermediate representation operation
  //
  class operation
  {
  public:
    using enum op_code;
    using byte_t = std::byte;
    using data_t = std::vector<byte_t>;
    using size_type = data_t::size_type;

  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(operation);

    ~operation() noexcept;

    operation(op_code opCode, size_type prealloc) noexcept;

  public:
    //
    // Returns the operation code
    //
    op_code code() const noexcept;

  private:
    data_t m_data;
  };
}