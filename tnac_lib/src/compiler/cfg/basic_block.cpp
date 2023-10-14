#include "compiler/cfg/basic_block.hpp"
#include "compiler/cfg/func.hpp"

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

  void basic_block::add_operation(op_t op) noexcept
  {
    m_operations.emplace_back(std::move(op));
  }


  // Private members

  void basic_block::add_inbound(cfg_edge& connection) noexcept
  {
    m_inbound.emplace_back(&connection);
  }

}