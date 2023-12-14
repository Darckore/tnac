#include "cfg/cfg.hpp"

namespace tnac::ir
{
  // Special members

  cfg::~cfg() noexcept = default;

  cfg::cfg(builder& bld) noexcept :
    m_builder{ &bld }
  {}


  // Public members

  ir::function* cfg::declare_module(semantics::module_sym& sym) noexcept
  {
    utils::unused(sym);
    return {};
  }

}