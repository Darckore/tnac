//
// Compiler
//

#pragma once
#include "parser/ast/ast_visitor.hpp"
#include "compiler/detail/context.hpp"
#include "compiler/detail/name_repo.hpp"
#include "compiler/detail/compiler_stack.hpp"

namespace tnac
{
  class sema;
  class feedback;

  namespace ir
  {
    class cfg;
    class edge;
  }

  namespace semantics
  {
    class symbol;
    class module_sym;
  }

  namespace eval
  {
    class store;
  }
}

namespace tnac
{
  //
  // Walks the AST and produces a control flow graph
  //
  class compiler final : public ast::bottom_up_visitor<compiler>
  {
  public:
    using tree     = ast::node;
    using tree_ptr = tree*;
    using tree_ref = tree&;

    using base_t = ast::bottom_up_visitor<compiler>;

    using params_t = std::vector<ast::param_decl*>;
    using body_t   = ast::scope::elem_list;

    using edge_view = std::span<ir::edge*>;
    using size_type = edge_view::size_type;
    using size_opt  = std::optional<size_type>;

    using val_opt = std::optional<eval::value>;

  public:
    CLASS_SPECIALS_NONE(compiler);

    ~compiler() noexcept;

    compiler(sema& sema, eval::store& valStore, ir::cfg& gr, feedback* fb) noexcept;

  public:
    //
    // Accepts an AST node and builds the corresponding CFG
    //
    void operator()(tree_ref node) noexcept;

  public:
    //
    // Returns a reference to the CFG
    //
    const ir::cfg& cfg() const noexcept;

    //
    // Returns a reference to the CFG
    //
    ir::cfg& cfg() noexcept;

    //
    // Looks at the top of the stack
    // If it holds a value, returns it
    // Otherwise, returns an empty optional
    //
    val_opt peek_value() const noexcept;

    //
    // Looks at the top of the stack
    // If it holds a register, returns a pointer to it
    // Otherwise, returns a nullptr
    //
    const ir::vreg* peek_reg() const noexcept;

    //
    // Compiles an existing module attached to the CFG after initial compilation
    // Also, stays in the newly attached module and maintains its context
    // Mostly, this is needed for REPL to function properly,
    // but can potentially be useful elswhere as well
    //
    void attach_module(ir::function& mod, ast::module_def& def) noexcept;

  public: // Exprs
    void visit(ast::error_expr& err) noexcept;
    void visit(ast::result_expr& res) noexcept;
    void visit(ast::ret_expr& ret) noexcept;
    void visit(ast::lit_expr& lit) noexcept;
    void visit(ast::id_expr& id) noexcept;
    void visit(ast::unary_expr& unary) noexcept;
    void visit(ast::tail_expr& tail) noexcept;
    void visit(ast::type_check_expr& tcheck) noexcept;
    void visit(ast::binary_expr& binary) noexcept;
    void visit(ast::array_expr& arr) noexcept;
    void visit(ast::abs_expr& abs) noexcept;
    void visit(ast::typed_expr& typed) noexcept;

    bool exit_child(ast::node& node) noexcept;
    void post_exit(ast::node& node) noexcept;

  public: // Decls
    void visit(ast::param_decl& param) noexcept;
    void visit(ast::func_decl& fd) noexcept;

  public: // Previews
    //
    // Inits the root
    //
    bool preview(ast::root& root) noexcept;

    //
    // Inits a function
    //
    bool preview(ast::func_decl& fd) noexcept;

    //
    // Declares and inits a variable
    //
    bool preview(ast::var_decl& var) noexcept;

    //
    // Compiles the assigned value and
    // generates a variable store instruction
    //
    bool preview(ast::assign_expr& assign) noexcept;

    //
    // Handles logical expressions
    //
    bool preview(ast::binary_expr& binary) noexcept;

    //
    // Handles type resolvers
    //
    bool preview(ast::type_resolve_expr& tres) noexcept;

    //
    // Handles short conditionals
    //
    bool preview(ast::cond_short& cond) noexcept;

    //
    // Handles conditionals
    //
    bool preview(ast::cond_expr& cond) noexcept;

    //
    // Used to report an error if a known function is provided with a wrong
    // number of arguments
    //
    bool preview(ast::call_expr& call) noexcept;

    //
    // Handles dot expressions with potentially unknown entities
    //
    bool preview(ast::dot_expr& dot) noexcept;

  private: // Emitions
    //
    // Updates the first instruction in the context if needed
    //
    void update_func_start(ir::instruction& instr) noexcept;

    //
    // Clears the saved store in context
    //
    void clear_store() noexcept;

    //
    // Converges two or more conditional branches together
    //
    void converge() noexcept;

    //
    // Prepares an instruction which has a result
    //
    ir::instruction& make(ir::op_code oc, size_opt prealloc = {}) noexcept;

    //
    // Creates an alloc instruction for the specified variable
    //
    void emit_alloc(semantics::symbol& sym) noexcept;

    //
    // Creates an alloc instruction for the specified variable name
    // Returns a reference to the created virtual register
    //
    ir::vreg& emit_alloc(string_t varName) noexcept;

    //
    // Creates an alloc instruction for an array of the given size
    // Returns a reference to the created virtual register
    //
    ir::vreg& emit_arr(ir::operand::idx_type size) noexcept;

    //
    // Creates a ret instruction
    //
    void emit_ret(ir::basic_block& block) noexcept;

    //
    // Creates a store instruction for the specified variable
    //
    void emit_store(semantics::symbol& var) noexcept;

    //
    // Creates a store instruction for the specified vreg and value
    //
    void emit_store(ir::vreg& target, ir::operand val) noexcept;

    //
    // Creates a load instruction for the specified variable
    //
    void emit_load(semantics::symbol& var) noexcept;

    //
    // Creates a load instruction for the specified vreg
    // Returns a reference to the register read into
    //
    ir::vreg& emit_load(ir::vreg& target) noexcept;

    //
    // Creates a binary instruction
    //
    void emit_binary(ir::op_code oc, ir::operand lhs, ir::operand rhs) noexcept;

    //
    // Creates a unary instruction
    //
    void emit_unary(ir::op_code oc, ir::operand val) noexcept;

    //
    // Creates a test instruction
    //
    void emit_test(ir::operand val, eval::type_id id) noexcept;

    //
    // Creates an unconditional jump instruction
    //
    void emit_jump(ir::operand value, ir::basic_block& dest) noexcept;

    //
    // Creates a conditional jump instruction
    //
    void emit_cond_jump(ir::operand cond, ir::basic_block& ifTrue, ir::basic_block& ifFalse, ir::operand falseV = eval::value{}) noexcept;

    //
    // Creates a phi node
    //
    void emit_phi(edge_view edges) noexcept;

    //
    // Creates a select instruction
    //
    void emit_select(ir::operand cond, ir::operand onTrue, ir::operand onFalse) noexcept;

    //
    // Creates an instance instruction
    //
    void emit_inst(ir::op_code oc, size_type opCount, size_type factCount) noexcept;

    //
    // Creates a series of append instructions to fill an array
    //
    void emit_append(ir::vreg& arr, size_type size) noexcept;

    //
    // Creates a series of instructions for a known array call
    //
    void emit_arr_call(ast::call_expr& call, eval::array_type arr) noexcept;

    //
    // Creates a call instruction
    //
    void emit_call(ir::operand callable, size_type argCount) noexcept;

    //
    // Creates a dynamic bind instruction
    //
    void emit_dyn(ir::operand scope, string_t name) noexcept;

  private:
    //
    // Creates a register for a variable and adds it to the context
    //
    ir::vreg& make_variable_register(string_t varName) noexcept;

    //
    // Deletes a basic block and all of the blocks it is connected to
    // Returns true on success
    //
    bool delete_block_tree(ir::basic_block& root) noexcept;

    //
    // Checks whether the current block has a connection to the return block
    // Needed to properly handle early returns
    //
    bool has_ret_jump() noexcept;

    //
    // Checks whether the given block has a connection to the return block
    // Needed to properly handle early returns
    //
    bool has_ret_jump(ir::basic_block& block) noexcept;

    //
    // Checks whether the previous instruction has been a jump, and we're in the same block
    //
    bool check_post_jmp() noexcept;

    //
    // Extracts the last values from the stack
    // If the stack is empty, loads the last stored value
    // If there's no last stored value, the result is undef
    //
    ir::operand extract() noexcept;

    //
    // Empties the stack on the next instruction
    //
    void empty_stack() noexcept;

    //
    // Emits a unary is to an operation's result,
    // unless the operation itself returns a bool already
    //
    void enforce_bool(const ir::operand& op) noexcept;

    //
    // Compiles an initialiser (rhs of var decls and assigns)
    //
    void compile_init(semantics::symbol& var, ast::expr& init) noexcept;

    //
    // Compiles a unary expression
    //
    void compile_unary(const ir::operand& op, tok_kind opType) noexcept;

    //
    // Compiles a unary expression
    //
    void compile_unary(const ir::operand& op, eval::val_ops opType, ir::op_code oc) noexcept;

    //
    // Compiles a type check expression
    //
    void compile_test(const ir::operand& op, eval::type_id id) noexcept;

    //
    // Compiles a binary expression
    //
    void compile_binary(const ir::operand& lhs, const ir::operand& rhs, tok_kind opType) noexcept;

    //
    // Compiles a pattern
    // Returns true if the pattern is known to match (when we have exact values)
    //
    bool compile(ast::pattern& pattern, const ir::operand& checked, bool last) noexcept;

    //
    // Compiles the implementation of a function or module
    //
    void compile(params_t& params, body_t& body) noexcept;

    //
    // Compiles a scope
    //
    void compile(ast::scope& scope) noexcept;

    //
    // Compiles a scope body
    //
    void compile(body_t& body) noexcept;

    //
    // Visits all subexpressions of the given expression and compiles functions
    // except anonimous ones
    // Returns true if any functions were encountered
    //
    bool compile_funcs(ast::node& expr) noexcept;

    //
    // Compiles the given module
    //
    void compile(semantics::module_sym& mod) noexcept;

    //
    // Walks the given ast node and compiles it
    //
    void compile(tree_ref node) noexcept;

    //
    // Loops until the module stack is empty and compiles modules one by one
    //
    void compile_modules() noexcept;

    //
    // Attempts to retrieve the module symbol from the given import directive
    //
    semantics::module_sym* get_module(ast::import_dir& imp) noexcept;

    //
    // Pushes module imports to the stack for further compilation
    // Returns false on error
    //
    bool walk_imports(ast::module_def& mod) noexcept;

    //
    // Registers imports as loose submodules
    //
    void register_import_scopes(ir::function& mod, ast::module_def& def) noexcept;

    //
    // Transfers last stored value in case there are multiple vars are assigned or declared
    // inside a single expression
    // The previous value that was on the stack is consumed at this point for a store operation
    //
    // The parameter holds a stack size value before the previous operation,
    // the transfer occurs if it is equal to the current stack size
    //
    void transfer_last_load(size_type prevSz) noexcept;

    //
    // Interns an array
    // This is needed in order to store compile-time arrays in a special data section
    //
    void intern_array(const ir::operand& op) noexcept;

    //
    // Interns arrays in all operands of an instruction
    //
    void intern_array(const ir::instruction& instr) noexcept;

    //
    // Recursively interns all subarrays of an array
    //
    void intern_array(eval::value val) noexcept;

    //
    // Reports a generic error
    //
    void error(string_t msg) noexcept;

    //
    // Reports an error tied to a src position
    //
    void error(src::loc_wrapper loc, string_t msg) noexcept;

    //
    // Reports a warning
    //
    void warning(src::loc_wrapper loc, string_t msg) noexcept;

    //
    // Posts a note
    //
    void note(src::loc_wrapper loc, string_t msg) noexcept;

  private:
    sema* m_sema{};
    feedback* m_feedback{};
    ir::cfg* m_cfg{};
    eval::store* m_vals{};
    detail::context m_context;
    detail::name_repo m_names;
    detail::compiler_stack m_stack;
  };
}