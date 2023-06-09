//
// Source lister
//

#pragma once
#include "output/common.hpp"

namespace tnac_rt::out
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

    void print(const ast::scope& scope) noexcept;

    void print(const ast::assign_expr& expr) noexcept;

    void print(const ast::decl_expr& expr) noexcept;

    void print(const ast::binary_expr& expr) noexcept;

    void print(const ast::unary_expr& expr) noexcept;

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

    void print(const ast::pattern& expr) noexcept;

    void print(const ast::matcher& expr) noexcept;
    
    void print(const ast::result_expr& expr) noexcept;

    void print(const ast::error_expr& expr) noexcept;

    void print(const ast::var_decl& decl) noexcept;
    
    void print(const ast::param_decl& decl) noexcept;
    
    void print(const ast::func_decl& decl) noexcept;

  private:
    using args_t = tnac::ast::list<tnac::ast::expr>::elem_list;

    void print_invocation(const ast::invocation& expr) noexcept;

    void print_args(const args_t& args, tnac::char_t open, tnac::char_t close) noexcept;

    void indent(const ast::node& cur) noexcept;

    void endl() noexcept;

    out_stream& out() noexcept;

    void print_token(const tnac::token& tok, bool addSpace) noexcept;

  private:
    out_stream* m_out{ &std::cout };
    static constexpr auto spacesPerIndent = std::size_t{ 2 };
    std::size_t m_indent{};
  };
}