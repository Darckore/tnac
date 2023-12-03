//
// Control flow graph
//

#pragma once
#include "cfg/ir/ir.hpp"

namespace tnac::ir
{
  class cfg final
  {
  public:
    CLASS_SPECIALS_NONE_CUSTOM(cfg);

    ~cfg() noexcept;

    cfg() noexcept = default; // temporary
  };
}