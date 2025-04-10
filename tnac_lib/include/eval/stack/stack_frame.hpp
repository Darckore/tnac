//
// Eval stack frame
//

#pragma once
#include "eval/value/value.hpp"

namespace tnac::eval
{
  //
  // Holds execution state for a function
  //
  class stack_frame final :
    public utils::ilist_node<stack_frame>
  {
  public:
    using param_count = std::uint16_t;
    using memory      = std::vector<value>;
    using name_type   = string_t;

  public:
    CLASS_SPECIALS_NONE(stack_frame);

    ~stack_frame() noexcept;

    stack_frame(name_type fname, param_count argSz, entity_id jmpBack) noexcept;

  public:
    //
    // Returns the name
    //
    name_type name() const noexcept;

    //
    // Appends a function argument and returns its id
    //
    stack_frame& add_arg(value argVal) noexcept;

    //
    // Allocates a variable and returns its id
    //
    entity_id allocate() noexcept;

    //
    // Returns the value assigned to a specific id
    //
    value value_for(entity_id id) const noexcept;

    //
    // Returns the jump back address
    //
    entity_id jump_back() const noexcept;

  private:
    memory m_mem;
    name_type m_name;
    entity_id m_jmp{};
  };
}