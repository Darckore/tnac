#include "input/repl.hpp"
#include "driver/state.hpp"

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

  }
}