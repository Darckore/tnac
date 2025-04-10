//
// Evaluator for the IR
//

#pragma once
#include "eval/stack/stack_frame.hpp"
#include "eval/value/value.hpp"
#include "cfg/cfg.hpp"

namespace tnac
{
  //
  // Runs evaluation over the CFG with the given input data
  //
  class ir_eval final
  {
  public:
    using call_stack = utils::stack<eval::stack_frame>;

  public:
    CLASS_SPECIALS_NONE(ir_eval);

    ~ir_eval() noexcept;

    explicit ir_eval(ir::cfg& cfg) noexcept;

  public:
    //
    // Creates a stack frame for the given function and enters it
    //
    void enter(const ir::function& func) noexcept;

  private:
    ir::cfg* m_cfg{};
    eval::value m_result{};
    call_stack m_stack;
  };
}