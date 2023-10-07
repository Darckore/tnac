#include "packages/pkg/cmd_pkg.hpp"

namespace tnac::packages
{
  // Special members

  cmd::~cmd() noexcept = default;

  cmd::cmd() noexcept :
    m_cmdInterpreter{ m_cmdStore }
  {}

  // Public members

  void cmd::on_command(value_type command) noexcept
  {
    m_cmdInterpreter.on_command(std::move(command));
  }

}