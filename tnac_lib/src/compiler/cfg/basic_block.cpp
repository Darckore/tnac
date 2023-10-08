#include "compiler/cfg/basic_block.hpp"

namespace tnac::comp
{
  // Basic block connection

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


  // Basic block

  // Special members

  basic_block::~basic_block() noexcept = default;

  basic_block::basic_block() noexcept = default;


  // Public members

  bb_conn& basic_block::add_outbound(basic_block& target) noexcept
  {
    return m_outbound.emplace_back(*this, target);
  }

}