//
// Simple expressions
//

#pragma once
#include "ir/ir.hpp"

namespace tnac::ir
{
  //
  // Constant
  //
  class constant final : public wrapper
  {
  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(constant);

    ~constant() noexcept;

    constant(reg_index_t reg, eval::value val) noexcept;
  };
}