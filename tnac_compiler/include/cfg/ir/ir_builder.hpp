//
// IR builder
//

#pragma once
#include "cfg/ir/ir.hpp"

namespace tnac::ir
{
  //
  // Creates and manages lifetime of IR nodes
  //
  class builder final
  {
  public:
    CLASS_SPECIALS_NONE_CUSTOM(builder);

    ~builder() noexcept;

    builder() noexcept;
  };
}