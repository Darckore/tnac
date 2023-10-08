#include "compiler/cfg/cfg.hpp"

namespace tnac::comp
{
  // Special members

  cfg::~cfg() noexcept = default;

  cfg::cfg() noexcept = default;

  cfg::operator bool() const noexcept
  {
    return !m_blocks.empty();
  }


  // Public members

  basic_block& cfg::entry() noexcept
  {
    UTILS_ASSERT(static_cast<bool>(*this));
    return *m_blocks.front();
  }

}