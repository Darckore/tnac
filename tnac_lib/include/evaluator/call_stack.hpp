//
// Call stack for functions
//

#pragma once
#include "evaluator/value/value.hpp"
#include "evaluator/value/value_visitor.hpp"
#include "ast/ast_nodes.hpp"
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
    using func_name  = std::string_view;
    using value_list = std::vector<value>;
    using sym_t      = semantics::function;
    using args_t     = ast::call_expr::arg_list;
    using vis_t      = eval::value_visitor;

    using frame = detail::stack_frame;
    using stack = std::vector<frame>;

  public:
    CLASS_SPECIALS_NONE(call_stack);

    ~call_stack() noexcept;

    explicit call_stack(size_type depth) noexcept;

    explicit operator bool() const noexcept;

  public:
    //
    // Saves values of function parameters
    //
    void push(const sym_t& callable, const args_t& args, vis_t& visitor) noexcept;

    //
    // Removes the most recent entry from the stack
    //
    void pop(vis_t& visitor) noexcept;

    //
    // Inits function parameters with stored temporaries
    //
    void prologue(sym_t& callable, vis_t& visitor) noexcept;

    //
    // Restores values of function parameters
    //
    void epilogue(sym_t& callable, vis_t& visitor) noexcept;

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