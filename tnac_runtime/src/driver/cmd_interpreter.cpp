#include "driver/cmd_interpreter.hpp"

namespace tnac_rt
{
  // Special members

  cmd::~cmd() noexcept = default;

  cmd::cmd(cmd_store& store) noexcept :
    m_cmdStore { store }
  {}

  // Public members

  void cmd::on_command(value_type command) noexcept
  {
    using enum commands::verification;
    auto descr = m_cmdStore.find(command.name());
    if (!descr)
    {
      on_error(command, WrongName);
      return;
    }

    // todo: verification
    descr->handler()(std::move(command));
  }

  // Private members

  void cmd::on_error(cmd_ref command, commands::verification reason) noexcept
  {
    if (!m_errHandler)
      return;

    using enum commands::verification;
    switch (reason)
    {
    case WrongName:
      m_errHandler(command.pos(), "Unknown command"sv);
      break;

    case WrongKind:
      break;

    case TooFew:
      break;

    case TooMany:
      break;

    default:
      UTILS_ASSERT(false);
      return;
    }
  }
}