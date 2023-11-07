#include "input/repl.hpp"
#include "driver/state.hpp"
#include "output/formatting.hpp"
#include "output/printer.hpp"
#include "output/lister.hpp"

namespace tnac::rt
{
  // Special members

  repl::~repl() noexcept = default;

  repl::repl(state& st) noexcept :
    m_state{ &st }
  {}


  // Public members

  void repl::run() noexcept
  {
    buf_t input;
    m_state->start();

    while (m_state->is_running())
    {
      m_state->out() << ">> ";
      std::getline(m_state->in(), input);
      if (utils::ltrim(input).empty())
      {
        m_state->out() << "Enter an expression\n";
        continue;
      }

      input = {};
    }
  }
}