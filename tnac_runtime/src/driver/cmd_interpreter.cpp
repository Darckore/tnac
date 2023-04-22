#include "driver/cmd_interpreter.hpp"

namespace tnac_rt
{
  // Special members

  cmd::~cmd() noexcept = default;

  cmd::cmd(cmd_store& store) noexcept :
    m_cmdStore { store }
  {}

  // Public members

  void cmd::on_command(value_type command) const noexcept
  {
    using enum commands::verification;
    auto foundCmd = m_cmdStore.find(command.name());
    if (!foundCmd)
    {
      on_error(command, { .m_res{ WrongName } });
      return;
    }

    auto&& descr = *foundCmd;
    
    if (auto ver = verify(command, descr); !ver)
    {
      on_error(command, ver);
      return;
    }

    descr(std::move(command));
  }

  // Private members

  cmd::ver_result cmd::verify(cmd_ref command, descr_ref base) const noexcept
  {
    ver_result res;

    using enum commands::verification;
    using size_type = ver_result::size_type;

    const auto paramSize = command.param_count();
    
    if (paramSize > base.size())
      res.m_res = TooMany;
    else if (paramSize < base.required())
      res.m_res = TooFew;

    if (res.m_res != Correct)
      res.m_diff = paramSize;
    else
    {
      auto idx = size_type{};
      for (; idx < paramSize; ++idx)
      {
        if (command[idx].is(base[idx]))
          continue;

        res.m_res = WrongKind;
        res.m_diff = idx;
        break;
      }
    }

    return res;
  }

  void cmd::on_error(cmd_ref command, const ver_result& reason) const noexcept
  {
    if (!m_errHandler)
      return;

    using enum commands::verification;
    tnac::string_t msg{};
    const tnac::token* pos{};

    switch (reason.m_res)
    {
    case WrongName:
      pos = &command.pos();
      msg = "Unknown command"sv;
      break;

    case TooFew:
      pos = !reason.m_diff ? &command.pos() : &command[reason.m_diff - 1];
      msg = "Too few parameters"sv;
      break;

    case TooMany:
      pos = &command[reason.m_diff - 1];
      msg = "Too many parameters"sv;
      break;

    case WrongKind:
      pos = &command[reason.m_diff];
      msg = "Wrong parameter type"sv;
      break;

    default:
      UTILS_ASSERT(false);
      return;
    }

    m_errHandler(*pos, msg);
  }
}