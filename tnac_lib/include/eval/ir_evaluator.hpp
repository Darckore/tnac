//
// Evaluator for the IR
//

#pragma once
#include "eval/stack/call_stack.hpp"
#include "eval/environment.hpp"
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
    using val_opt = std::optional<eval::value>;

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
    // Clears the environment
    //
    void clear_env() noexcept;

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

    //
    // Steps into the next instruction
    // Returns false if the end is reached
    //
    bool step() noexcept;

  private:
    //
    // Returns a reference to the current instruction
    //
    const ir::instruction& cur() const noexcept;

    //
    // Attempts to extract a value from the given operand
    //
    val_opt get_value(const ir::operand& op) const noexcept;

    //
    // Locates a register id
    //
    entity_id get_reg(const ir::vreg& reg) const noexcept;

    //
    // Dispatches the current instruction and moves the instuction pointer
    // Result is stored on the current stack frame
    //
    void dispatch() noexcept;

    //
    // Allocates a variable
    //
    void alloc() noexcept;

    //
    // Stores a value from a store instruction
    //
    void store() noexcept;

  private:
    ir::cfg* m_cfg{};
    eval::store* m_valStore{};
    eval::env m_env;
    eval::value m_result{};
    eval::call_stack m_stack;
    eval::stack_frame* m_curFrame{};
    const ir::instruction* m_instrPtr{};
  };
}