//
// IR
//

#pragma once
#include "eval/value/value.hpp"

namespace tnac::ir
{
  //
  // Operation code for the IR
  //
  enum class op_code : std::uint8_t
  {
#include "general/op_codes.inl"
  };

  //
  // Intermediate representation operation
  //
  class operation
  {
  public:
    using enum op_code;
    using byte_t      = std::byte;
    using data_t      = std::span<byte_t>;
    using size_type   = data_t::size_type;
    using reg_index_t = std::uint16_t;

  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(operation);

    ~operation() noexcept;

    explicit operation(data_t data) noexcept;

  public:
    //
    // Returns the operation code
    //
    op_code code() const noexcept;

  private:
    data_t m_data;
  };
}