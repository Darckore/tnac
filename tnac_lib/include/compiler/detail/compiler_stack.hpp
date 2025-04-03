//
// Compiler stack
//

#pragma once
#include "cfg/ir/ir_instructions.hpp"

namespace tnac::detail
{
  template <typename F>
  concept op_processor = std::is_nothrow_invocable_r_v<void, F, ir::operand>;

  //
  // Operates on values used in compilation
  //
  class compiler_stack final
  {
  public:
    using value_type = ir::operand;
    using data_type  = std::vector<value_type>;
    using size_type  = data_type::size_type;

    template <size_type N>
    using val_array = std::array<eval::value, N>;

    template <size_type N>
    using cval_array = const val_array<N>;

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
    // Returns the current stack size
    //
    size_type size() const noexcept;

    //
    // Checks whether the stack has the given number of known values at the top
    //
    bool has_values(size_type count) const noexcept;

    //
    // Checks whether the stack has at least the given number of items
    //
    bool has_at_least(size_type count) const noexcept;

    //
    // Fills operands to the instruction
    // Operands are read in reverse order (offset to top)
    //
    void fill(ir::instruction& instr, size_type count) noexcept;

    //
    // Fills values to the given array
    // Operands are read in reverse order (offset to top)
    //
    //void fill(arr_type& arr, size_type count) noexcept;

    //
    // Constructs a typed value from the current stack contents
    // and pushes it back on the stack
    //
    void instantiate(eval::type_id type, size_type argSz) noexcept;

  private:
    //
    // Walks the specified values on the stack in reverse order
    // and applies the given function to each
    //
    void walk_back(size_type count, op_processor auto&& proc) noexcept;

    //
    // Extracts the specified number of values and constructs a value of the given type
    // using the extracted ones as its arguments
    //
    template <eval::expr_result Obj>
    void instantiate(size_type argSz) noexcept;

    //
    // Instantiation helper for types
    //
    template <eval::expr_result Obj, typename Int, Int... Seq>
    void instantiate(cval_array<sizeof...(Seq)>& args, utils::idx_seq<Int, Seq...>) noexcept;

  private:
    data_type m_data;
  };
}