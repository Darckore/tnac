#include "driver/cmd_interpreter.hpp"

namespace tnac_rt
{
  // Special members

  cmd::~cmd() noexcept = default;

  cmd::cmd() noexcept = default;

  // Public members

  void cmd::on_command(tnac::ast::command command) noexcept
  {
    utils::unused(command);
  }
}