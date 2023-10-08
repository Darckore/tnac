//
// CFG builder
//

#pragma once
#include "parser/ast/ast_visitor.hpp"
#include "compiler/cfg/cfg.hpp"

namespace tnac::comp
{
  //
  // Walks the AST and creates a control-flow graph
  //
  class cfg_builder final : public ast::bottom_up_visitor<cfg_builder>
  {
  public:
    using base = ast::bottom_up_visitor<cfg_builder>;
    
    using block_name = basic_block::name_t;
    using name_idx   = std::uintmax_t;

  private:
    //
    // Retrieves the index used in generated names
    //
    static name_idx gen_name_idx() noexcept;

    //
    // Generates a name for scope
    //
    static block_name make_name(ast::scope& scope) noexcept;

  public:
    CLASS_SPECIALS_NONE(cfg_builder);

    ~cfg_builder() noexcept;
    explicit cfg_builder(cfg& gr) noexcept;

    void operator()(ast::node* root) noexcept;

  public:
    //
    // Returns a reference to the control-flow graph worked on
    //
    cfg& get() noexcept;

  public: // Expressions
    //
    // Visits a literal expression
    //
    void visit(ast::lit_expr& lit) noexcept;

    //
    // Visits a unary expression
    //
    void visit(ast::unary_expr& unary) noexcept;

    //
    // Visits a binary expression
    //
    void visit(ast::binary_expr& binary) noexcept;

  public: // Previews
    //
    // Creates a basic block for a scope
    //
    bool preview(ast::scope& scope) noexcept;

  private:
    cfg* m_cfg;
  };
}