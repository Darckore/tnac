//
// Evaluator for the IR
//

#pragma once
#include "eval/stack/call_stack.hpp"
#include "eval/value/value.hpp"
#include "eval/value/value_store.hpp"
#include "cfg/cfg.hpp"

namespace tnac
{
  //
  // Runs evaluation over the CFG with the given input data
  //
  class ir_eval final
  {
  public:
    CLASS_SPECIALS_NONE(ir_eval);

    ~ir_eval() noexcept;

    ir_eval(ir::cfg& cfg, eval::store& vals) noexcept;

  public:
    //
    // Returns the evaluation result
    //
    eval::value result() const noexcept;

    //
    // Creates a stack frame for the given function and enters it
    //
    void enter(const ir::function& func) noexcept;

    //
    // Leaves the current function
    //
    void leave() noexcept;

    //
    // Evaluates an entire function
    //
    void evaluate_current() noexcept;

  private:
    ir::cfg* m_cfg{};
    eval::store* m_valStore{};
    eval::value m_result{};
    eval::call_stack m_stack;
    eval::stack_frame* m_curFrame{};
    const ir::instruction* m_instrPtr{};
  };
}