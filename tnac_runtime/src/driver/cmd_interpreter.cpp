#include "driver/cmd_interpreter.hpp"

namespace tnac_rt
{
  // Special members

  cmd::~cmd() noexcept = default;

  cmd::cmd() noexcept = default;

  // Public members

  void cmd::on_command(value_type command) noexcept
  {
    using enum commands::cmd_id;
    const auto cmdId = m_classifier.verify(command);
    
    if (cmdId == Unknown)
    {
      on_error(command);
      return;
    }
  }

  // Private members

  void cmd::on_error(cmd_ref command) noexcept
  {
    if (!m_errHandler)
      return;

    auto verRes = m_classifier.last_result();
    using enum value_type::verification;

    switch (verRes.m_res)
    {
    case Correct:
      m_errHandler(command.pos(), "Unrecognised command"sv);
      break;

    case TooFew:
      m_errHandler(command.pos(), "Too few parameters"sv);
      break;

    case TooMany:
      m_errHandler(command.pos(), "Too many parameters"sv);
      break;

    case WrongKind:
      m_errHandler(command.pos(), "Wrong parameter type"sv);
      break;

    default:
      break;
    }
  }
}