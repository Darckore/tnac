#include "ast/ast_util.hpp"

namespace tnac::ast
{
  // Error expr

  error_expr::~error_expr() noexcept = default;

  error_expr::error_expr(const token& tok, string_t msg) noexcept :
    expr{ kind::Error, tok },
    m_errMsg{ msg }
  {}

  const token& error_expr::at() const noexcept
  {
    return pos();
  }

  string_t error_expr::message() const noexcept
  {
    return m_errMsg;
  }

}