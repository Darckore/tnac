//
// Call stack
//

#pragma once
#include "eval/stack/stack_frame.hpp"

namespace tnac::eval
{
  //
  // Manages stack frames for functions
  //
  class call_stack final
  {
  public:
    using data_type   = stack_frame::list_type;
    using param_count = stack_frame::param_count;
    using name_type   = stack_frame::name_type;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(call_stack);

    ~call_stack() noexcept;

    call_stack() noexcept;

  public:
    //
    // Creates a new stack frame and returns a reference to it
    //
    stack_frame& make_frame(name_type name, param_count argSz, entity_id jmp) noexcept;

    //
    // Removes the most recent stack frame and returns a pointer to a previous one
    //
    stack_frame* pop_frame() noexcept;

  private:
    data_type m_frames;
  };
}