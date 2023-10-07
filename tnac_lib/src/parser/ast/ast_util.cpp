#include "parser/ast/ast_util.hpp"

namespace tnac::ast
{
  // Error expr

  error_expr::~error_expr() noexcept = default;

  error_expr::error_expr(const token& tok, string_t msg) noexcept :
    expr{ kind::Error, tok },
    m_errMsg{ msg }
  {
    make_invalid();
  }

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

  command::command(const token& cmd, arg_list args) noexcept :
    m_cmd{ cmd },
    m_args{ std::move(args) }
  {}

  const token& command::operator[](size_type idx) const noexcept
  {
    return m_args[idx];
  }

  const token& command::pos() const noexcept
  {
    return m_cmd;
  }

  string_t command::name() const noexcept
  {
    return pos().value();
  }

  command::size_type command::arg_count() const noexcept
  {
    return m_args.size();
  }

  command::iterator command::begin() const noexcept
  {
    return m_args.begin();
  }

  command::iterator command::end() const noexcept
  {
    return m_args.end();
  }
}