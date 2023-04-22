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

    if (verRes.m_res == Correct)
      return;

    tnac::string_t msg{};
    const tnac::token* pos{};

    switch (verRes.m_res)
    {
    case WrongName:
      pos = &command.pos();
      msg = "Unrecognised command"sv;
      break;

    case TooFew:
      pos = !verRes.m_diff ? &command.pos() : &command[verRes.m_diff - 1];
      msg = "Too few parameters"sv;
      break;

    case TooMany:
      pos = &command[verRes.m_diff - 1];
      msg = "Too many parameters"sv;
      break;

    case WrongKind:
      pos = &command[verRes.m_diff];
      msg = "Wrong parameter type"sv;
      break;

    default:
      UTILS_ASSERT(false);
      return;
    }

    m_errHandler(*pos, msg);
  }
}