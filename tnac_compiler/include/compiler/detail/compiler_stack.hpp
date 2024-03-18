//
// Compiler stack
//

#pragma once
#include "cfg/ir/ir_instructions.hpp"

namespace tnac::detail
{
  //
  // Operates on values used in compilation
  //
  class compiler_stack final
  {
  public:
    using value_type = ir::operand;
    using data_type  = std::vector<value_type>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(compiler_stack);

    ~compiler_stack() noexcept;

    compiler_stack() noexcept;

  public:
    //
    // Pushes an operand to the stack
    //
    void push(ir::operand op) noexcept;

    //
    // Returns the current top item
    //
    value_type top() noexcept;

    //
    // Removes the top item
    //
    void pop() noexcept;

    //
    // Pops the top item and removes it from the stack
    //
    value_type extract() noexcept;

    //
    // Checks whether the stack has any data
    //
    bool empty() const noexcept;

  private:
    data_type m_data;
  };
}