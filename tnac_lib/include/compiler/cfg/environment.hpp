//
// Environment
//

#pragma once

namespace tnac::comp
{
  //
  // Environment
  // 
  // Used to manage registers and variable access
  //
  class environment final
  {
  public:
    using reg_index_t = std::uint16_t;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(environment);

    ~environment() noexcept;

    environment() noexcept;
  };
}