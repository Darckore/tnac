#include "ir/ir.hpp"

namespace tnac::ir // operation
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

  void operation::add_value(eval::value val) noexcept
  {
    eval::on_value(val, utils::visitor
    {
      [this](auto&&) {},
      [this](eval::int_type i) { add_value(i); },
      [this](eval::float_type f) { add_value(f); },
      [this](eval::complex_type c) { add_value(c); }
    });
  }


  // Private members

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

  void operation::add_type_id(eval::type_id ti) noexcept
  {
    m_data.emplace_back(std::bit_cast<byte_t>(ti));
  }

}

namespace tnac::ir // ir_wrapper
{
  // Special members

  wrapper::~wrapper() noexcept = default;

  wrapper::wrapper(op_code code, size_type prealloc) noexcept :
    m_op{ code, prealloc }
  {}


  // Public members

  [[nodiscard]]
  wrapper::op_t wrapper::operator*() && noexcept
  {
    return std::move(op());
  }


  // Protected members

  wrapper::op_t& wrapper::op() noexcept
  {
    return m_op;
  }

}