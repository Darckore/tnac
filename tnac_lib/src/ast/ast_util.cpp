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


  // Command

  command::~command() noexcept = default;

  command::command(const token& cmd, param_list params) noexcept :
    m_cmd{ cmd },
    m_params{ std::move(params) }
  {}

  const token& command::operator[](size_type idx) const noexcept
  {
    return m_params[idx];
  }

  const token& command::pos() const noexcept
  {
    return m_cmd;
  }

  string_t command::name() const noexcept
  {
    return pos().m_value;
  }

  command::size_type command::param_count() const noexcept
  {
    return m_params.size();
  }

  command::iterator command::begin() const noexcept
  {
    return m_params.begin();
  }

  command::iterator command::end() const noexcept
  {
    return m_params.end();
  }
}