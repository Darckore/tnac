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
    using data_t      = std::vector<byte_t>;
    using size_type   = data_t::size_type;
    using reg_index_t = std::uint16_t;

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
    // Adds a value operand
    //
    void add_value(eval::value val) noexcept;

    //
    // Adds a virtula register index to store the result in
    //
    void add_register(reg_index_t idx) noexcept;

  private:
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


  //
  // A wrapper around IR for convenient access
  //
  class wrapper
  {
  public:
    using op_t        = operation;
    using byte_t      = op_t::byte_t;
    using op_view     = std::span<byte_t>;
    using size_type   = op_t::size_type;
    using reg_index_t = op_t::reg_index_t;

  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(wrapper);

    ~wrapper() noexcept;

    wrapper(op_code code, size_type prealloc) noexcept;

  public:
    [[nodiscard]]
    op_t operator*() && noexcept;

  protected:
    op_t& op() noexcept;

  private:
    operation m_op;
  };

  template <typename T>
  concept op_wrapper = std::derived_from<T, wrapper>;
}