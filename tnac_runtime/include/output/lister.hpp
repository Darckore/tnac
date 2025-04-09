//
// Source lister
//

#pragma once
#include "output/common.hpp"

namespace tnac::rt::out
{
  //
  // Prints the source code corresponding to an ast node
  //
  class lister final
  {
  public:
    CLASS_SPECIALS_NONE_CUSTOM(lister);

    ~lister() noexcept;

    lister() noexcept;

  public:
    void operator()(const ast::node* node, out_stream& os) noexcept;

    void operator()(const ast::node* node) noexcept;

  private:
    void print(const ast::node* root) noexcept;

    void print(const ast::root& root) noexcept;
    
    void print(const ast::module_def& mod) noexcept;

    void print(const ast::import_dir& id) noexcept;

    void print(const ast::scope& scope) noexcept;

    void print(const ast::assign_expr& expr) noexcept;

    void print(const ast::decl_expr& expr) noexcept;

    void print(const ast::binary_expr& expr) noexcept;

    void print(const ast::unary_expr& expr) noexcept;

    void print(const ast::type_check_expr& expr) noexcept;
    
    void print(const ast::tail_expr& expr) noexcept;

    void print(const ast::array_expr& expr) noexcept;

    void print(const ast::paren_expr& expr) noexcept;

    void print(const ast::abs_expr& expr) noexcept;

    void print(const ast::typed_expr& expr) noexcept;

    void print(const ast::call_expr& expr) noexcept;

    void print(const ast::lit_expr& expr) noexcept;

    void print(const ast::id_expr& expr) noexcept;

    void print(const ast::ret_expr& expr) noexcept;

    void print(const ast::cond_short& expr) noexcept;

    void print(const ast::cond_expr& expr) noexcept;

    void print(const ast::dot_expr& expr) noexcept;

    void print(const ast::pattern& expr) noexcept;

    void print(const ast::matcher& expr) noexcept;

    void print(const ast::result_expr& expr) noexcept;

    void print(const ast::error_expr& expr) noexcept;

    void print(const ast::var_decl& decl) noexcept;

    void print(const ast::param_decl& decl) noexcept;

    void print(const ast::func_decl& decl) noexcept;

  private:
    using args_t   = ast::list<ast::expr>::elem_list;
    using params_t = std::vector<ast::param_decl*>;

    const ast::node& nearest_to_scope(const ast::node& src) noexcept;

    void print_invocation(const ast::invocation& expr) noexcept;

    void print_args(const args_t& args, char_t open, char_t close) noexcept;

    void print_params(const params_t& params) noexcept;

    void indent(const ast::node& cur) noexcept;

    void endl() noexcept;

    out_stream& out() noexcept;

    void print_token_plain(const token& tok, bool addSpace) noexcept;

    void print_token(const token& tok, bool addSpace) noexcept;

    void reset_style() noexcept;
    void id_style() noexcept;
    void kw_style() noexcept;
    void lit_style() noexcept;
    void default_style() noexcept;
    void comment_style() noexcept;

  private:
    out_stream* m_out{ &std::cout };
    static constexpr auto spacesPerIndent = std::size_t{ 2 };
    std::size_t m_indent{};
  };
}