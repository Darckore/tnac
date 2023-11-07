//
// REPL
//

#pragma once

namespace tnac::rt
{
  class state;

  //
  // REPL shell
  // Provides interactive input
  //
  class repl final
  {
  public:
    CLASS_SPECIALS_NONE(repl);

    ~repl() noexcept;

    explicit repl(state& st) noexcept;

  public:
    //
    // Runs the REPL
    //
    void run() noexcept;

  private:
    state* m_state{};
  };
}