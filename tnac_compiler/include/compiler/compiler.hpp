//
// Compiler
//

#pragma once
#include "parser/ast/ast_visitor.hpp"
#include "cfg/cfg.hpp"

namespace tnac
{
  class sema;
  class feedback;
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

  public:
    CLASS_SPECIALS_NONE(compiler);

    ~compiler() noexcept;

    compiler(sema& sema, feedback* fb) noexcept;

  public:
    //
    // Accepts an AST node and builds the corresponding CFG
    //
    void operator()(tree_ref node) noexcept;

  public:
    //
    // Returns a reference to the CFG
    // 
    // const version
    //
    const ir::cfg& cfg() const noexcept;

    //
    // Returns a reference to the CFG
    //
    ir::cfg& cfg() noexcept;

  public: // General
    void visit(ast::root& root) noexcept;
    void visit(ast::module_def& mod) noexcept;
    void visit(ast::import_dir& imp) noexcept;
    void visit(ast::scope& scope) noexcept;
    void visit(ast::error_expr& err) noexcept;

  public: // Exprs
    void visit(ast::result_expr& res) noexcept;
    void visit(ast::ret_expr& ret) noexcept;
    void visit(ast::lit_expr& lit) noexcept;
    void visit(ast::id_expr& id) noexcept;
    void visit(ast::unary_expr& unary) noexcept;
    void visit(ast::binary_expr& binary) noexcept;
    void visit(ast::assign_expr& assign) noexcept;
    void visit(ast::decl_expr& decl) noexcept;
    void visit(ast::array_expr& arr) noexcept;
    void visit(ast::paren_expr& paren) noexcept;
    void visit(ast::abs_expr& abs) noexcept;
    void visit(ast::typed_expr& typed) noexcept;
    void visit(ast::call_expr& call) noexcept;
    void visit(ast::matcher& matcher) noexcept;
    void visit(ast::pattern& pattern) noexcept;
    void visit(ast::cond_short& cond) noexcept;
    void visit(ast::cond_expr& cond) noexcept;
    void visit(ast::dot_expr& dot) noexcept;

  public: // Decls
    void visit(ast::var_decl& var) noexcept;
    void visit(ast::param_decl& param) noexcept;
    void visit(ast::func_decl& func) noexcept;

  private:
    sema* m_sema{};
    feedback* m_feedback{};
    ir::cfg m_cfg;
  };
}