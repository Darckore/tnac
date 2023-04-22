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
  class lister
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

    void print(const ast::paren_expr& expr) noexcept;

    void print(const ast::lit_expr& expr) noexcept;

    void print(const ast::id_expr& expr) noexcept;

    void print(const ast::result_expr& expr) noexcept;

    void print(const ast::error_expr& expr) noexcept;

    void print(const ast::var_decl& expr) noexcept;

  private:
    void endl() noexcept;

    out_stream& out() noexcept;

    void print_token(const tnac::token& tok, bool addSpace) noexcept;

  private:
    out_stream* m_out{ &std::cout };
  };
}