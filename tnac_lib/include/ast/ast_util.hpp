//
// Nodes for utility expressions
//

#pragma once
#include "ast/ast_expr.hpp"

namespace tnac::ast
{
  //
  // Error expr
  //
  class error_expr : public expr
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(error_expr);

    virtual ~error_expr() noexcept;

  protected:
    error_expr(const token& tok, string_t msg) noexcept;

  public:
    const token& at() const noexcept;

    string_t message() const noexcept;

  private:
    string_t m_errMsg;
  };

}