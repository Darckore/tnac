//
// Control flow graph
//

#pragma once
#include "cfg/ir/ir_builder.hpp"
#include "sema/sym/symbols.hpp"

namespace tnac::ir
{
  //
  // Control flow graph of the program
  // Provides access to the IR
  //
  class cfg final
  {
  public:
    CLASS_SPECIALS_NONE(cfg);

    ~cfg() noexcept;

    explicit cfg(builder& bld) noexcept;

  public:
    //
    // Declares a module and enters it
    // If the module already exists, just enters it
    //
    void enter_module(semantics::module_sym& mod) noexcept;

    //
    // Exits the currently compiled module
    //
    void exit_module() noexcept;

  private:
    builder* m_builder;
  };
}