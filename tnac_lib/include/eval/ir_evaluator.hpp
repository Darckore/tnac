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
  class feedback;
}

namespace tnac
{
  //
  // Runs evaluation over the CFG with the given input data
  //
  class ir_eval final
  {
  private:
    struct branch
    {
      const ir::basic_block* m_from{};
      const ir::basic_block* m_to{};
    };

    struct arr_call
    {
      std::size_t m_idx{};
      entity_id m_callRes{};
    };

    using branch_stack = utils::stack<branch>;
    using arr_map      = std::unordered_map<eval::array_wrapper*, arr_call>;

  public:
    using val_opt  = std::optional<eval::value>;
    using op_count = ir::instruction::size_type;

  public:
    CLASS_SPECIALS_NONE(ir_eval);

    ~ir_eval() noexcept;

    ir_eval(ir::cfg& cfg, eval::store& vals, feedback* fb) noexcept;

  public:
    //
    // Attempts to load a value from the given register
    // If successful, overwrites the last result
    //
    void try_load(const ir::vreg& reg) noexcept;

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
    // Inits the instruction pointer with the provided instruction
    //
    void init_instr_ptr(const ir::instruction& instr) noexcept;

    //
    // Returns the current instruction pointer
    //
    const ir::instruction* instr_ptr() const noexcept;

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
    // Attempts to extract a value from the given operand
    //
    val_opt get_value(const eval::stack_frame& frame, const ir::operand& op) const noexcept;

    //
    // Locates a register id
    //
    entity_id get_reg(const ir::vreg& reg) const noexcept;

    //
    // Locates a register id
    //
    entity_id get_reg(const eval::stack_frame* frame, const ir::vreg& reg) const noexcept;

    //
    // Stores a value to the specified register
    //
    void store_value(entity_id reg, const ir::operand& from) noexcept;

    //
    // Stores a value to the specified register
    //
    void store_value(eval::stack_frame& frame, entity_id reg, const ir::operand& from) noexcept;

    //
    // Stores a value to the specified register
    //
    void store_value(entity_id reg, eval::value val) noexcept;

    //
    // Stores a value to the specified register
    //
    void store_value(eval::stack_frame& frame, entity_id reg, eval::value val) noexcept;

    //
    // Creates a new stack value and returns its id
    //
    entity_id alloc_new(entity_id op) noexcept;

    //
    // Creates a new stack value and returns its id
    // Creates a new stack value and returns its id
    //
    entity_id alloc_new(const ir::operand& op) noexcept;

    //
    // Enters the specified basic block,
    // sets the instruction pointer, and updates the current branch
    //
    void jump_to(const ir::operand& op) noexcept;

    //
    // Enters the specified basic block,
    // sets the instruction pointer, and updates the current branch
    //
    void jump_to(const ir::basic_block& block) noexcept;

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
    // Allocates an array
    //
    void alloc_array() noexcept;

    //
    // Appends a value into an array
    //
    void append() noexcept;

    //
    // Stores a value from a store instruction
    //
    void store() noexcept;

    //
    // Loads a value from a load instruction
    //
    void load() noexcept;

    //
    // Handles jumps
    //
    void jump() noexcept;

    //
    // Handles phi nodes
    //
    void phi() noexcept;

    //
    // Handles dynamic binds
    //
    void dyn_bind() noexcept;

    //
    // Calculates a select op
    //
    void select() noexcept;

    //
    // Calculates a unary
    //
    void unary(ir::op_code oc) noexcept;

    //
    // Calculates a binary
    //
    void binary(ir::op_code oc) noexcept;

    //
    // Tests the type of a value
    //
    void test_type() noexcept;

    //
    // Instantiates types
    //
    void type(ir::op_code oc) noexcept;

    //
    // Calls the target function
    // Returns true on success
    //
    bool call(entity_id regId, eval::value f, const ir::instruction& instr) noexcept;

    //
    // Performs an array call
    // Returns true is the array has not been exhausted yet
    //
    bool call(entity_id regId, eval::array_wrapper& arr, const ir::instruction& instr) noexcept;

    //
    // Calls the target function
    //
    void call() noexcept;

    //
    // Handles returns from functions
    //
    void ret();

  private:
    ir::cfg* m_cfg{};
    eval::store* m_valStore{};
    eval::env m_env;
    eval::value m_result{};
    eval::call_stack m_stack;
    eval::stack_frame* m_curFrame{};
    branch_stack m_branching;
    arr_map m_arrCalls;
    const ir::instruction* m_instrPtr{};
    feedback* m_feedback{};
  };
}