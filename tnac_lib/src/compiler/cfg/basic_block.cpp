#include "compiler/cfg/basic_block.hpp"

namespace tnac::comp // Basic block connection
{
  // Special members

  bb_conn::~bb_conn() noexcept = default;

  bb_conn::bb_conn(basic_block& from, basic_block& to) noexcept :
    m_from{ &from },
    m_to{ &to }
  {}


  // Public members

  basic_block& bb_conn::source() noexcept
  {
    return *m_from;
  }
  basic_block& bb_conn::dest() noexcept
  {
    return *m_to;
  }

}

namespace tnac::comp // Basic block
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

  bb_conn& basic_block::add_outbound(basic_block& target) noexcept
  {
    auto&& res = *m_outbound.emplace_back(std::make_unique<bb_conn>(*this, target));
    target.add_inbound(res);
    return res;
  }


  // Private members

  void basic_block::add_inbound(bb_conn& connection) noexcept
  {
    m_inbound.emplace_back(&connection);
  }

}