#include "input/cmdline.hpp"

namespace tnac::rt
{
  // Special members

  cmdline::~cmdline() noexcept = default;

  cmdline::cmdline() noexcept = default;


  // Public members

  void cmdline::parse(int argCount, char** args) noexcept
  {
    m_state = {};

    if (argCount < 2)
    {
      m_state.m_interactive = true;
      return;
    }

    m_state.m_inputFile = args[1];

    for (auto argIdx = 2; argIdx < argCount; ++argIdx)
    {
      tnac::string_t arg = args[argIdx];
      consume(arg);
    }
  }

  bool cmdline::has_input_file() const noexcept
  {
    return !run_on().empty();
  }

  cmdline::name_t cmdline::run_on() const noexcept
  {
    return m_state.m_inputFile;
  }

  bool cmdline::interactive() const noexcept
  {
    return m_state.m_interactive;
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
      m_state.m_interactive = true;
    else
      error("Unknown cli arg");
  }
}