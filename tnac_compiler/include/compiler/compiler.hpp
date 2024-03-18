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
    void visit(ast::assign_expr& assign) noexcept;
    void visit(ast::array_expr& arr) noexcept;
    void visit(ast::abs_expr& abs) noexcept;
    void visit(ast::typed_expr& typed) noexcept;
    void visit(ast::call_expr& call) noexcept;
    void visit(ast::matcher& matcher) noexcept;
    void visit(ast::pattern& pattern) noexcept;
    void visit(ast::cond_short& cond) noexcept;
    void visit(ast::cond_expr& cond) noexcept;
    void visit(ast::dot_expr& dot) noexcept;

  public: // Decls
    void visit(ast::param_decl& param) noexcept;

  public: // Previews
    //
    // Inits the root
    //
    bool preview(ast::root& root) noexcept;

    //
    // Inits a function
    //
    bool preview(ast::func_decl& fd) noexcept;

    bool preview(ast::var_decl& var) noexcept;

    bool preview(ast::assign_expr& assign) noexcept;

  private: // Emitions
    //
    // Updates the first instruction in the context if needed
    //
    void update_context(ir::instruction& instr) noexcept;

    //
    // Creates an alloc instruction for the specified variable
    //
    ir::vreg& emit_alloc(string_t varName) noexcept;

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
    // Creates a binary arithmetic instruction
    //
    void emit_binary(ir::op_code oc, ir::operand lhs, ir::operand rhs) noexcept;

  private:
    //
    // Transfers a calculated value to the internal stack
    //
    void carry_val(entity_id id) noexcept;

    //
    // Compiles the implementation of a function or module
    //
    void compile(params_t& params, body_t& body) noexcept;

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