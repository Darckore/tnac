//
// Compiler
//

#pragma once
#include "parser/ast/ast_visitor.hpp"
#include "eval/value/evaluator.hpp"
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

  namespace eval
  {
    class registry;
  }

  namespace semantics
  {
    class symbol;
    class module_sym;
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

  public:
    CLASS_SPECIALS_NONE(compiler);

    ~compiler() noexcept;

    compiler(sema& sema, ir::cfg& gr, eval::registry& reg, feedback* fb) noexcept;

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

  public: // Exprs
    void visit(ast::error_expr& err) noexcept;
    void visit(ast::result_expr& res) noexcept;
    void visit(ast::ret_expr& ret) noexcept;
    void visit(ast::lit_expr& lit) noexcept;
    void visit(ast::id_expr& id) noexcept;
    void visit(ast::unary_expr& unary) noexcept;
    void visit(ast::binary_expr& binary) noexcept;
    void visit(ast::array_expr& arr) noexcept;
    void visit(ast::abs_expr& abs) noexcept;
    void visit(ast::typed_expr& typed) noexcept;
    void visit(ast::call_expr& call) noexcept;
    void visit(ast::dot_expr& dot) noexcept;

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
    // Handles short conditionals
    //
    bool preview(ast::cond_short& cond) noexcept;

    //
    // Handles conditionals
    //
    bool preview(ast::cond_expr& cond) noexcept;

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
    // Creates a ret instruction
    //
    void emit_ret(ir::basic_block& block) noexcept;

    //
    // Creates a store instruction for the specified variable
    //
    void emit_store(semantics::symbol& var) noexcept;

    //
    // Creates a load instruction for the specified variable
    //
    void emit_load(semantics::symbol& var) noexcept;

    //
    // Creates a binary instruction
    //
    void emit_binary(ir::op_code oc, ir::operand lhs, ir::operand rhs) noexcept;

    //
    // Creates a unary instruction
    //
    void emit_unary(ir::op_code oc, ir::operand val) noexcept;

    //
    // Creates an unconditional jump instruction
    //
    void emit_jump(ir::operand value, ir::basic_block& dest) noexcept;

    //
    // Creates a conditional jump instruction
    //
    void emit_cond_jump(ir::operand cond, ir::basic_block& ifTrue, ir::basic_block& ifFalse) noexcept;

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

  private:
    //
    // Extracts the last values from the stack
    // If the stack is empty, loads the last stored value
    // If there's no last stored value, the result is undef
    //
    ir::operand extract() noexcept;

    //
    // Transfers a calculated value to the internal stack
    //
    void carry_val(entity_id id) noexcept;

    //
    // Empties the stack on the next instruction
    //
    void empty_stack() noexcept;

    //
    // Compiles a unary expression
    //
    void compile_unary(entity_id expr, const ir::operand& val, tok_kind opType) noexcept;

    //
    // Compiles a binary expression
    //
    void compile_binary(entity_id expr, const ir::operand& lhs, const ir::operand& rhs, tok_kind opType) noexcept;

    //
    // Compiles a pattern
    // Returns true if the pattern is known to match (when we have exact values)
    //
    bool compile(ast::pattern& pattern, const ir::operand& checked) noexcept;

    //
    // Compiles the implementation of a function or module
    //
    void compile(params_t& params, body_t& body) noexcept;

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
    eval::evaluator m_eval;
    detail::context m_context;
    detail::name_repo m_names;
    detail::compiler_stack m_stack;
  };
}