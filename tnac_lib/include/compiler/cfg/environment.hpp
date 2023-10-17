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
    using reg_stack   = std::stack<reg_index_t>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(environment);

    ~environment() noexcept;

    environment() noexcept;

  public:
    //
    // Returns a virtual register index to use in IR generation
    // Also, pushes it on the stack for later use
    //
    [[nodiscard]]
    reg_index_t next_register() noexcept;

    //
    // Removes the last stored register index from the stack and returns it
    //
    [[nodiscard]]
    reg_index_t pop_register() noexcept;

  private:
    reg_stack m_regStack;
    reg_index_t m_curRegisterIdx{};
  };
}