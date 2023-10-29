#include "compiler/cfg/basic_block.hpp"
#include "compiler/cfg/func.hpp"

namespace tnac::comp
{
  namespace detail
  {
    template <typename Val>
    concept ir_const = tnac::is_any_v<Val,
      eval::bool_type,
      eval::int_type,
      eval::float_type,
      eval::complex_type
    >;

    template <typename Val>
    concept invalid_ir_const = !ir_const<Val>;
  }
}

namespace tnac::comp
{
  // Special members

  basic_block::~basic_block() noexcept = default;

  basic_block::basic_block(name_t name, func& parent) noexcept :
    m_name{ std::move(name) },
    m_parent{ &parent }
  {}


  // Public members

  string_t basic_block::name() const noexcept
  {
    return m_name;
  }

  const func& basic_block::parent() const noexcept
  {
    return *m_parent;
  }

  func& basic_block::parent() noexcept
  {
    return FROM_CONST(parent);
  }

  cfg_edge& basic_block::add_outbound(basic_block& target) noexcept
  {
    auto&& res = *m_outbound.emplace_back(std::make_unique<cfg_edge>(*this, target));
    target.add_inbound(res);
    return res;
  }


  // Public members(Operations)

  void basic_block::add_constant(reg_index_t saveTo, eval::value val) noexcept
  {
    alloc_op(sizeof(reg_index_t) + value_size(val));
    add_operand(ir::op_code::Constant);
    add_register(saveTo);
    add_value(val);
  }

  void basic_block::add_binary(ir::op_code code, reg_index_t saveTo, reg_index_t left, reg_index_t right) noexcept
  {
    alloc_op(sizeof(reg_index_t) * 3);
    add_operand(code);
    add_register(saveTo);
    add_register(left);
    add_register(right);
  }


  // Private members

  void basic_block::add_inbound(cfg_edge& connection) noexcept
  {
    m_inbound.emplace_back(&connection);
  }

  void basic_block::alloc_op(size_type size) noexcept
  {
    const auto newSize = sizeof(ir::op_code) + size + m_opData.size();
    m_opData.reserve(newSize);
  }

  basic_block::size_type basic_block::value_size(eval::value val) const noexcept
  {
    return sizeof(eval::type_id) + val.size();
  }

  void basic_block::add_value(eval::value val) noexcept
  {
    eval::on_value(val, utils::visitor
      {
        [this](detail::invalid_ir_const auto&&) {},
        [this](detail::ir_const auto v) { add_value(v); }
      });
  }

  void basic_block::add_register(reg_index_t idx) noexcept
  {
    add_operand(idx);
  }

  void basic_block::add_value(eval::bool_type val) noexcept
  {
    add_type_id(eval::type_id::Bool);
    add_operand(val);
  }

  void basic_block::add_value(eval::int_type val) noexcept
  {
    add_type_id(eval::type_id::Int);
    add_operand(val);
  }

  void basic_block::add_value(eval::float_type val) noexcept
  {
    add_type_id(eval::type_id::Float);
    add_operand(val);
  }

  void basic_block::add_value(eval::complex_type val) noexcept
  {
    add_type_id(eval::type_id::Complex);
    add_operand(val.real());
    add_operand(val.imag());
  }

  void basic_block::add_type_id(eval::type_id ti) noexcept
  {
    m_opData.emplace_back(std::bit_cast<byte_t>(ti));
  }


}