//
// Compilation context
//

#pragma once

namespace tnac
{
  namespace ast
  {
    class module_def;
  }

  namespace semantics
  {
    class module_sym;
  }

  namespace ir
  {
    class function;
    class basic_block;
    class instruction;
  }
}

namespace tnac::detail
{
  //
  // Holds associations between module symbols and their corresponding ASTs
  // Also, provides context to the compiler
  //
  class context final
  {
  public:
    using module_sym   = semantics::module_sym;
    using module_def   = ast::module_def;
    using data_store   = std::unordered_map<module_sym*, module_def*>;
    using module_stack = std::vector<module_sym*>;
    using block_queue  = std::queue<ir::basic_block*>;
    using instr_iter   = utils::ilist<ir::instruction>::iterator;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(context);

    ~context() noexcept;

    context() noexcept;

  public:
    //
    // Appends a sym-tree association
    //
    void store(module_sym& sym, module_def& def) noexcept;

    //
    // Returns the AST node corresponding to a module
    //
    module_def* locate(module_sym& sym) noexcept;

    //
    // Discards the collected data
    //
    void wipe() noexcept;

    //
    // Pushes a module symbol on the stack
    //
    void push(module_sym& sym) noexcept;

    //
    // Pops the next module symbol from the stack
    // Returns nullptr if empty
    //
    module_sym* pop() noexcept;

    //
    // Sets the given function as the current one
    //
    void enter_function(ir::function& fn) noexcept;

    //
    // Resets the current function to its owner
    // If the current function is a module, acts as exit_module
    //
    void exit_function() noexcept;

    //
    // Returns a reference to the current function
    // Use with caution and check whether one exists beforehand
    //
    ir::function& current_function() noexcept;

    //
    // Adds a basic block
    //
    ir::basic_block& create_block(string_t name) noexcept;

    //
    // Adds the given basic block to the queue
    //
    void enqueue_block(ir::basic_block& block) noexcept;

    //
    // Returns the basic block from the top of the queue
    //
    ir::basic_block& current_block() noexcept;

    //
    // Returns a pointer to the terminal (last) block of the function
    //
    ir::basic_block* terminal_block() noexcept;

    //
    // Returns a reference to the terminal block if available
    // Otherwise, returns a reference to the entry
    //
    ir::basic_block& terminal_or_entry() noexcept;

    //
    // Sets the terminal block of the current function
    //
    void terminate_at(ir::basic_block& term) noexcept;

    //
    // Pops the current basic block
    //
    void exit_block() noexcept;

    //
    // Updates the first instruction of the function
    // If the stored iterator is already valid, does nothing
    //
    void func_start_at(ir::instruction& instr) noexcept;

    //
    // Gets the first instruction of the current function
    //
    instr_iter funct_start() noexcept;

  private:
    //
    // Clears data associated with the current function
    //
    void drop_func() noexcept;

  private:
    data_store m_data;
    module_stack m_stack;
    block_queue m_blocks;
    ir::function* m_curFunction{};
    instr_iter m_funcFirst{};
    ir::basic_block* m_terminal{};
  };
}