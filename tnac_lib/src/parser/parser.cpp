#include "parser/parser.hpp"

namespace tnac
{
  // Public members

  parser::pointer parser::parse(string_t str) noexcept
  {
    m_lex.feed(str);

    auto eList = expression_list();
    if (eList.empty())
      return {};

    auto res = eList.back();
    if (!m_root)
    {
      m_root = m_builder.make_scope({}, std::move(eList));
    }

    return res;
  }

  parser::const_root_ptr parser::root() const noexcept
  {
    return m_root;
  }
  parser::root_ptr parser::root() noexcept
  {
    return utils::mutate(std::as_const(*this).root());
  }

  // Private members

  parser::expr_list parser::expression_list() noexcept
  {
    expr_list res;

    while (auto e = expr())
    {
      res.push_back(e);
    }

    return res;
  }

  ast::expr* parser::expr() noexcept
  {
    return primary_expr();
  }

  ast::expr* parser::primary_expr() noexcept
  {
    auto&& tok = m_lex.peek();
    if (tok.is_literal())
    {
      return m_builder.make_literal({}, m_lex.next());
    }

    return {};
  }
}