#include "cfg/cfg.hpp"

namespace tnac::ir
{
  // Special members

  cfg::~cfg() noexcept = default;

  cfg::cfg(builder& bld) noexcept :
    m_builder{ &bld }
  {}


  // Public members

  void cfg::enter_module(semantics::module_sym& mod) noexcept
  {
    utils::unused(mod);
  }

  void cfg::exit_module() noexcept
  {

  }

}