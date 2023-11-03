//
// REPL
//

#pragma once

namespace tnac::rt
{
  //
  // REPL shell
  // Provides interactive input
  //
  class repl final
  {
  public:
    CLASS_SPECIALS_NONE_CUSTOM(repl);

    ~repl() noexcept;

    repl() noexcept;

  public:
    //
    // Runs the REPL
    //
    void run() noexcept;
  };
}