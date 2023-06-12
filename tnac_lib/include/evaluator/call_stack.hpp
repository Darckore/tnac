//
// Call stack for functions
//

#pragma once
#include "evaluator/value.hpp"

namespace tnac::eval
{
  namespace detail
  {
    //
    // A call stack entry
    //
    class stack_frame;
  }

  //
  // Manages the call stack
  //
  class call_stack final
  {
  public:
    using size_type  = std::size_t;
    using func_name  = std::string_view;
    using value_list = std::vector<value>;

    using frame = detail::stack_frame;
    using stack = std::vector<frame>;

  public:
    CLASS_SPECIALS_NONE(call_stack);

    ~call_stack() noexcept;

    explicit call_stack(size_type depth) noexcept;

    explicit operator bool() const noexcept;

  public:
    //
    // Attempts to push a function to the stack
    // Returns true on success, false if overflown
    //
    bool push(func_name funcName, value_list&& funcArgs) noexcept;

    //
    // Removes the most recent entry from the stack
    //
    void pop() noexcept;

  private:
    //
    // Checks whether adding a new frame would overflow the stack
    //
    bool can_push() const noexcept;

  private:
    size_type m_depth{};
    stack m_frames;
  };
}