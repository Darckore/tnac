//
// AST printer
//

#pragma once
#include "output/common.hpp"

namespace tnac::rt::out
{
  //
  // Printer for AST
  // Visits each node top to bottom and prints it into the specified stream
  //
  class ast_printer final : public ast::const_top_down_visitor<ast_printer>
  {
  public:
    using base = ast::const_top_down_visitor<ast_printer>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(ast_printer);

    ~ast_printer() noexcept;

    ast_printer() noexcept;

  public:
    void operator()(const ast::node* node, out_stream& os) noexcept;

    void operator()(const ast::node* node) noexcept;

  public:
    void visit(const ast::root& root) noexcept;

    void visit(const ast::module_def& moduleDef) noexcept;

    void visit(const ast::import_dir& id) noexcept;

    void visit(const ast::scope& scope) noexcept;

    void visit(const ast::assign_expr& expr) noexcept;

    void visit(const ast::decl_expr& expr) noexcept;

    void visit(const ast::var_decl& decl) noexcept;

    void visit(const ast::param_decl& decl) noexcept;

    void visit(const ast::func_decl& decl) noexcept;

    void visit(const ast::binary_expr& expr) noexcept;

    void visit(const ast::unary_expr& expr) noexcept;

    void visit(const ast::tail_expr& expr) noexcept;

    void visit(const ast::array_expr& expr) noexcept;

    void visit(const ast::paren_expr& expr) noexcept;

    void visit(const ast::abs_expr& expr) noexcept;

    void visit(const ast::typed_expr& expr) noexcept;

    void visit(const ast::call_expr& expr) noexcept;

    void visit(const ast::cond_short& expr) noexcept;

    void visit(const ast::cond_expr& expr) noexcept;

    void visit(const ast::dot_expr& expr) noexcept;

    void visit(const ast::pattern& ptrn) noexcept;

    void visit(const ast::matcher& matcher) noexcept;

    void visit(const ast::lit_expr& expr) noexcept;

    void visit(const ast::id_expr& expr) noexcept;

    void visit(const ast::result_expr& expr) noexcept;

    void visit(const ast::ret_expr& expr) noexcept;

    void visit(const ast::error_expr& expr) noexcept;

  private:
    using child_count = std::size_t;
    using child_tracker = std::vector<child_count>;

  private:
    void push_parent(child_count childCount) noexcept;

    void pop_empty() noexcept;

    child_count& last_indent() noexcept;

    void print_parent(child_count cur) noexcept;

    void print_child(child_count cur) noexcept;

    void indent() noexcept;

    void node_designator(string_t str) noexcept;

    void failure_condition(string_t str) noexcept;

    void node_value(string_t str) noexcept;

    void module_name(string_t str) noexcept;

    void invalid_mark(const ast::node& n) noexcept;

    void location_info(src::loc_wrapper loc) noexcept;

    void additional_info(const ast::node& n) noexcept;

    void additional_info(const ast::expr& e) noexcept;

    void additional_info(const ast::decl& d) noexcept;
    
    void additional_info(const ast::import_dir& d) noexcept;

    void endl() noexcept;

    out_stream& out() noexcept;

    void print_token(const token& tok) noexcept;

    void print_value(const eval::value& v) noexcept;

    void print_module_name(const ast::module_def& moduleDef) noexcept;

  private:
    child_tracker m_indetations;
    out_stream* m_out{ &std::cout };
  };
}