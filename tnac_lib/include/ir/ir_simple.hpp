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

    explicit constant(eval::value val) noexcept;
  };
}