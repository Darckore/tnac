//
// AST printer
//

#pragma once
#include "ast/ast_visitor.hpp"

namespace tnac_rt::out
{
  namespace ast = tnac::ast;

  //
  // Printer base class
  //
  template <typename D>
  using printer_base = ast::const_top_down_visitor<D>;

  //
  // Printer for AST
  // Visits each node top to bottom and prints it into the specified stream
  //
  class ast_printer : public printer_base<ast_printer>
  {
  public:
    CLASS_SPECIALS_NONE_CUSTOM(ast_printer);

    ~ast_printer() noexcept;

    ast_printer() noexcept;

    void operator()(const ast::node* node, out_stream& os) noexcept;

  private:
    using base = printer_base<ast_printer>;
    using indent_t  = unsigned;
    using child_count = std::size_t;

    struct indent_descr
    {
      child_count m_childIdx{};
      indent_t    m_depth{};
    };

    using indent_stack = std::stack<indent_descr>;

    static constexpr auto indentStep = 2u;

  private:
    void push_parent(child_count childCount) noexcept;

    void indent() noexcept;

    void endl() noexcept;

    out_stream& out() noexcept;

    void print_token(const tnac::token& tok) noexcept;

    void print_value(tnac::eval::value v) noexcept;

  public:
    void visit(const ast::scope* scope) noexcept;

    void visit(const ast::assign_expr* expr) noexcept;

    void visit(const ast::decl_expr* expr) noexcept;

    void visit(const ast::var_decl* decl) noexcept;

    void visit(const ast::binary_expr* expr) noexcept;

    void visit(const ast::unary_expr* expr) noexcept;

    void visit(const ast::paren_expr* expr) noexcept;

    void visit(const ast::lit_expr* expr) noexcept;

    void visit(const ast::id_expr* expr) noexcept;

    void visit(const ast::error_expr* expr) noexcept;

  private:
    indent_stack m_indetations;
    out_stream* m_out{};
    indent_t m_curDepth{};
  };
}