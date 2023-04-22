//
// AST printer
//

#pragma once
#include "output/common.hpp"

namespace tnac_rt::out
{
  //
  // Printer for AST
  // Visits each node top to bottom and prints it into the specified stream
  //
  class ast_printer final : public printer_base<ast_printer>
  {
  public:
    CLASS_SPECIALS_NONE_CUSTOM(ast_printer);

    ~ast_printer() noexcept;

    ast_printer() noexcept;

  public:
    void operator()(const ast::node* node, out_stream& os) noexcept;
    
    void operator()(const ast::node* node) noexcept;

  public:
    void visit(const ast::scope& scope) noexcept;

    void visit(const ast::assign_expr& expr) noexcept;

    void visit(const ast::decl_expr& expr) noexcept;

    void visit(const ast::var_decl& decl) noexcept;

    void visit(const ast::binary_expr& expr) noexcept;

    void visit(const ast::unary_expr& expr) noexcept;

    void visit(const ast::paren_expr& expr) noexcept;

    void visit(const ast::lit_expr& expr) noexcept;

    void visit(const ast::id_expr& expr) noexcept;

    void visit(const ast::result_expr& expr) noexcept;

    void visit(const ast::error_expr& expr) noexcept;

  private:
    using base = printer_base<ast_printer>;
    using child_count = std::size_t;
    using child_tracker = std::vector<child_count>;

  private:
    void push_parent(child_count childCount) noexcept;

    void pop_empty() noexcept;

    child_count& last_indent() noexcept;

    void print_parent(child_count cur) noexcept;

    void print_child(child_count cur) noexcept;

    void indent() noexcept;

    void endl() noexcept;

    out_stream& out() noexcept;

    void print_token(const tnac::token& tok) noexcept;

    void print_value(eval::value v) noexcept;

  private:
    child_tracker m_indetations;
    out_stream* m_out{ &std::cout };
  };
}