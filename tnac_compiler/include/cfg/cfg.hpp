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
    // Declares a new module
    //
    ir::function* declare_module(semantics::module_sym& sym) noexcept;

  private:
    builder* m_builder;
  };
}