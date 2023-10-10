#include "compiler/cfg/edge.hpp"

namespace tnac::comp
{
  // Special members

  cfg_edge::~cfg_edge() noexcept = default;

  cfg_edge::cfg_edge(basic_block& from, basic_block& to) noexcept :
    m_from{ &from },
    m_to{ &to }
  {}


  // Public members

  basic_block& cfg_edge::source() noexcept
  {
    return *m_from;
  }
  basic_block& cfg_edge::dest() noexcept
  {
    return *m_to;
  }

}