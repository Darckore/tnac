#include "driver/cmd_interpreter.hpp"

namespace tnac_rt
{
  // Special members

  cmd::~cmd() noexcept = default;

  cmd::cmd() noexcept = default;

  // Public members

  void cmd::on_command(value_type command) noexcept
  {
    utils::unused(command);
  }

  // Private members

  /*
    Old code from command:

  command::verification_result command::verify(const descr& base) const noexcept
  {
    verification_result res;

    const auto paramSize = param_count();

    if (base.m_name != name())
      res.m_res = WrongName;
    else if (paramSize > base.m_params.size())
      res.m_res = TooMany;
    else if (paramSize < base.m_reqCount)
      res.m_res = TooFew;

    if (res.m_res != Correct)
      res.m_diff = paramSize;
    else
    {
      auto idx = size_type{};
      for ( ; idx < paramSize; ++idx)
      {
        if((*this)[idx].is(base.m_params[idx]))
          continue;

        res.m_res = WrongKind;
        res.m_diff = idx;
        break;
      }
    }

    return res;
  }
  */

  void cmd::on_error(cmd_ref command) noexcept
  {
    utils::unused(command);
    //if (!m_errHandler)
    //  return;

    //auto verRes = m_classifier.last_result();
    //using enum value_type::verification;

    //if (verRes.m_res == Correct)
    //  return;

    //tnac::string_t msg{};
    //const tnac::token* pos{};

    //switch (verRes.m_res)
    //{
    //case WrongName:
    //  pos = &command.pos();
    //  msg = "Unrecognised command"sv;
    //  break;

    //case TooFew:
    //  pos = !verRes.m_diff ? &command.pos() : &command[verRes.m_diff - 1];
    //  msg = "Too few parameters"sv;
    //  break;

    //case TooMany:
    //  pos = &command[verRes.m_diff - 1];
    //  msg = "Too many parameters"sv;
    //  break;

    //case WrongKind:
    //  pos = &command[verRes.m_diff];
    //  msg = "Wrong parameter type"sv;
    //  break;

    //default:
    //  UTILS_ASSERT(false);
    //  return;
    //}

    //m_errHandler(*pos, msg);
  }
}