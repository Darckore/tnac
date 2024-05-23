//
// Compilation context
//

#pragma once

namespace tnac
{
  namespace ast
  {
    class module_def;
    class ret_expr;
    class scope;
  }

  namespace semantics
  {
    class symbol;
    class module_sym;
  }

  namespace ir
  {
    class function;
    class basic_block;
    class instruction;
    class vreg;
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
    using instr_iter   = utils::ilist<ir::instruction>::iterator;
    using symbol       = semantics::symbol;
    using reg_idx      = std::uint64_t;

  private:
    struct var_data;
    using var_store       = std::unordered_map<symbol*, var_data>;
    using known_var_names = std::unordered_set<string_t>;
    struct func_data;
    using data_stack = std::vector<func_data>;

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
    // Appends a symbol-register association
    //
    void store(symbol& sym, ir::vreg& reg) noexcept;

    //
    // Returns the register referring to a variable storage
    //
    ir::vreg* locate(symbol& sym) noexcept;

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
    void enter_function(ir::function& fn, ast::scope& fnScope) noexcept;

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
    // Sets the given block as the one used for explicit return calls
    // Also, sets the dedicated return variable
    //
    void set_return(ir::basic_block& retBlock, ir::vreg& retVal) noexcept;

    //
    // Sets the given basic block as current
    //
    void enter_block(ir::basic_block& block) noexcept;

    //
    // Returns the current basic block
    //
    ir::basic_block& current_block() noexcept;

    //
    // Returns a pointer to the terminal (last) block of the function
    //
    ir::basic_block* terminal_block() noexcept;

    //
    // Returns a pointer to the dedicated return block
    //
    ir::basic_block* return_block() noexcept;

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

    //
    // Gets the first instruction of the next function
    //
    instr_iter func_end() noexcept;

    //
    // Returns the current register index and increments it
    //
    reg_idx register_index() noexcept;

    //
    // Saves the last stored variable
    //
    void save_store(symbol& var) noexcept;

    //
    // Clears the last stored variable
    //
    void clear_store() noexcept;

    //
    // Returns the last store if available
    //
    symbol* last_store() noexcept;

    //
    // Marks a variable as read
    //
    void read_into(symbol& var, ir::vreg& reg) noexcept;

    //
    // Marks a variable as modified
    //
    void modify(symbol& var) noexcept;

    //
    // Returns the last register a variable was read to
    //
    ir::vreg* last_read(symbol& var) noexcept;

    //
    // Returns a pointer to the dedicated return variable
    //
    ir::vreg* ret_val() noexcept;

    //
    // Allows overriding the position used to add instruction (func last)
    // Returns the old one
    //
    instr_iter override_last(instr_iter newPos) noexcept;

    //
    // Remembers an existing var name for the current function
    // Returns false if the name already exists
    //
    bool new_var_name(string_t name) noexcept;

    //
    // Sets the explicit ret expression
    //
    void attach_ret(ast::ret_expr& expr) noexcept;

    //
    // Resets the explicit ret expression
    //
    void detach_ret() noexcept;

    //
    // Returns the set explicit ret expression
    //
    ast::ret_expr* explicit_ret() const noexcept;

    //
    // Enters the given scope
    // Returns a reference to the previous scope
    //
    ast::scope& enter_scope(ast::scope& scope) noexcept;

    //
    // Checks whether the current scope is the same as the function scope
    //
    bool is_in_func_scope() const noexcept;

  private:
    //
    // Returns function data from the top of the data stack
    //
    const func_data& cur_data() const noexcept;

    //
    // Returns function data from the top of the data stack
    //
    func_data& cur_data() noexcept;

    //
    // Locates a variable and returns its descriptor
    //
    var_data* locate_var(symbol& sym) noexcept;

  private:
    data_store m_data;
    module_stack m_stack;
    data_stack m_funcs;
  };
}