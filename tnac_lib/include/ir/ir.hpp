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

    //
    // Adds an int operand
    //
    void add_value(eval::int_type val) noexcept;

    //
    // Adds a float operand
    //
    void add_value(eval::float_type val) noexcept;

    //
    // Adds a complex operand
    //
    void add_value(eval::complex_type val) noexcept;

  private:
    //
    // Adds a type id prefix to the operand
    //
    void add_type_id(eval::type_id ti) noexcept;

    //
    // Adds a value operand
    //
    template <typename T>
    void add_operand(T val) noexcept
    {
      constexpr auto dataSize = sizeof(val);
      std::array<byte_t, dataSize> byteArr{};
      std::memcpy(byteArr.data(), &val, dataSize);
      m_data.append_range(byteArr);
    }

  private:
    data_t m_data;
  };
}