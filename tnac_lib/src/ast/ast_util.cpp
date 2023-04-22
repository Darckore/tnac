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

  command::verification_result command::verify(const descr& base) const noexcept
  {
    verification_result res;

    const auto paramSize = param_count();

    if (base.m_name != name())
      res.m_res = WrongName;
    else if (paramSize > base.m_params.size())
      res.m_res = TooMany;
    else if (paramSize < base.m_reqCount)
      res.m_res = TooFew;

    if (res.m_res != Correct)
      res.m_diff = paramSize;
    else
    {
      auto idx = size_type{};
      for ( ; idx < paramSize; ++idx)
      {
        if((*this)[idx].is(base.m_params[idx]))
          continue;

        res.m_res = WrongKind;
        res.m_diff = idx;
        break;
      }
    }

    return res;
  }

}