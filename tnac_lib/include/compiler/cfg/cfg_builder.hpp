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

  public:
    CLASS_SPECIALS_NONE(cfg_builder);

    ~cfg_builder() noexcept;
    explicit cfg_builder(cfg& gr) noexcept;

    void operator()(ast::node* root) noexcept;

  public:
    //
    // Returns a reference to the control-flow graph worked on
    //
    cfg& get_cfg() noexcept;

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

  private:
    cfg* m_cfg;
  };
}