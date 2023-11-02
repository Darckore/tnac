//
// Call stack for functions
//

#pragma once
#include "eval/value/value.hpp"
#include "eval/value/value_visitor.hpp"
#include "parser/ast/ast.hpp"
#include "sema/symbol.hpp"

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
    using value_list = std::vector<stored_value>;
    using sym_t      = semantics::function;
    using var_t      = semantics::symbol;
    using vis_t      = eval::value_visitor;
    using args_t     = value_list;

    using frame = detail::stack_frame;
    using stack = std::vector<frame>;

  public:
    CLASS_SPECIALS_NONE(call_stack);

    ~call_stack() noexcept;

    explicit call_stack(size_type depth) noexcept;

    explicit operator bool() const noexcept;

  public:
    //
    // Saves previous variable value in the current stack frame on assignment
    //
    void store_var(var_t& sym, stored_value prev) noexcept;

    //
    // Saves values of function parameters and arguments
    //
    void push(const sym_t& callable, const args_t& args, vis_t& visitor) noexcept;

    //
    // Inits function parameters with stored temporaries
    //
    void prologue(const sym_t& callable, vis_t& visitor) noexcept;

    //
    // Restores values of function parameters
    //
    void epilogue(const sym_t& callable, vis_t& visitor) noexcept;

    //
    // Clears the call stack
    //
    void clear() noexcept;

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