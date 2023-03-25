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
      m_root = m_builder.make_scope(std::move(eList));
    }
    else
    {
      m_root->adopt(std::move(eList));
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
    return unary_expr();
  }

  ast::expr* parser::unary_expr() noexcept
  {
    auto&& tok = m_lex.peek();
    if(!tok.is_any(token::Plus, token::Minus))
      return primary_expr();

    auto op = m_lex.next();
    if (auto exp = primary_expr())
    {
      return m_builder.make_unary(*exp, op);
    }

    return {};
  }

  ast::expr* parser::primary_expr() noexcept
  {
    auto&& tok = m_lex.peek();
    if (tok.is_literal())
    {
      return m_builder.make_literal(m_lex.next());
    }

    return {};
  }
}