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
    using size_type  = data_type::size_type;

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
    // Pushes an undefined value to the stack
    //
    void push_undef() noexcept;

    //
    // Returns the current top item
    //
    value_type top() noexcept;

    //
    // Removes the top item
    //
    void pop() noexcept;

    //
    // Discards the specified number of items from the stack
    //
    void drop(size_type count) noexcept;

    //
    // Saves the top item and removes it from the stack
    // Then returns the extracted value
    //
    value_type extract() noexcept;

    //
    // Extracts the item from the top of the stack
    // If the stack is empty, returns an undefined value
    //
    value_type try_extract() noexcept;

    //
    // Checks whether the stack has any data
    //
    bool empty() const noexcept;

    //
    // Checks whether the stack has the given number of known values at the top
    //
    bool has_values(size_type count) const noexcept;

  private:
    data_type m_data;
  };
}