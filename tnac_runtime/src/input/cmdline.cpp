#include "input/cmdline.hpp"

namespace tnac_rt
{
  // Special members

  cmdline::~cmdline() noexcept = default;

  cmdline::cmdline(int argCount, char** args) noexcept
  {
    parse(argCount, args);
  }


  // Public members

  bool cmdline::interactive() const noexcept
  {
    return m_interactive;
  }


  // Private members

  void cmdline::error(tnac::string_t msg) noexcept
  {
    if (m_errHandler)
      m_errHandler(msg);
  }

  void cmdline::consume(tnac::string_t arg) noexcept
  {
    if (arg == "-i"sv)
      m_interactive = true;
  }

  void cmdline::parse(int argCount, char** args) noexcept
  {
    if (argCount < 2)
    {
      m_interactive = true;
      return;
    }

    m_inputFile = args[1];

    for (auto argIdx = 2; argIdx < argCount; ++argIdx)
    {
      tnac::string_t arg = args[argIdx];
      consume(arg);
    }
  }
}