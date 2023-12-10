#include "cfg/cfg.hpp"

namespace tnac::ir
{
  // Special members

  cfg::~cfg() noexcept = default;

  cfg::cfg(builder& bld) noexcept :
    m_builder{ &bld }
  {}


}