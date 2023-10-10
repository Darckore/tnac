#include "compiler/cfg/basic_block.hpp"

namespace tnac::comp
{
  // Special members

  basic_block::~basic_block() noexcept = default;

  basic_block::basic_block(name_t name) noexcept :
    m_name{ std::move(name) }
  {}


  // Public members

  string_t basic_block::name() const noexcept
  {
    return m_name;
  }

  cfg_edge& basic_block::add_outbound(basic_block& target) noexcept
  {
    auto&& res = *m_outbound.emplace_back(std::make_unique<cfg_edge>(*this, target));
    target.add_inbound(res);
    return res;
  }


  // Private members

  void basic_block::add_inbound(cfg_edge& connection) noexcept
  {
    m_inbound.emplace_back(&connection);
  }

}