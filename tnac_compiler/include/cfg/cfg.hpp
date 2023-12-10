//
// Control flow graph
//

#pragma once
#include "cfg/ir/ir_builder.hpp"

namespace tnac::ir
{
  class cfg final
  {
  public:
    CLASS_SPECIALS_NONE(cfg);

    ~cfg() noexcept;

    explicit cfg(builder& bld) noexcept;

  private:
    builder* m_builder;
  };
}