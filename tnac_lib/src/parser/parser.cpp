#include "parser/parser.hpp"

namespace tnac
{
  namespace detail
  {
    namespace
    {
      constexpr auto is_unary_op(const token& tok) noexcept
      {
        return tok.is_any(token::Plus, token::Minus);
      }
      constexpr auto is_add_op(const token& tok) noexcept
      {
        return tok.is_any(token::Plus, token::Minus);
      }
      constexpr auto is_mul_op(const token& tok) noexcept
      {
        return tok.is_any(token::Asterisk, token::Slash);
      }

      constexpr auto is_open_paren(const token& tok) noexcept
      {
        return tok.is(token::ParenOpen);
      }
      constexpr auto is_close_paren(const token& tok) noexcept
      {
        return tok.is(token::ParenClose);
      }

      constexpr auto is_expression_separator(const token& tok) noexcept
      {
        return tok.is(token::ExprSep);
      }
    }
  }

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

  const token& parser::peek_next() noexcept
  {
    return m_lex.peek();
  }

  void parser::to_expr_end() noexcept
  {
    for (;;)
    {
      auto next = peek_next();
      if (next.is_eol() || detail::is_expression_separator(next))
        break;

      m_lex.next();
    }
  }

  ast::expr* parser::error_expr(string_t msg) noexcept
  {
    auto pos = m_lex.next();
    to_expr_end();
    return m_builder.make_error(pos, msg);
  }

  parser::expr_list parser::expression_list() noexcept
  {
    expr_list res;

    while (auto e = expr())
    {
      res.push_back(e);
      
      auto&& next = peek_next();
      if (next.is_eol())
        break;

      if (detail::is_expression_separator(next))
      {
        m_lex.next();
        continue;
      }

      res.push_back(error_expr("Expected ':' or EOL"));
    }

    return res;
  }

  ast::expr* parser::expr() noexcept
  {
    return additive_expr();
  }

  ast::expr* parser::additive_expr() noexcept
  {
    auto lhs = multiplicative_expr();
    if (lhs && detail::is_add_op(peek_next()))
    {
      auto op = m_lex.next();
      if (auto rhs = additive_expr())
        return m_builder.make_binary(*lhs, *rhs, op);
    }

    return lhs;
  }

  ast::expr* parser::multiplicative_expr() noexcept
  {
    auto lhs = unary_expr();
    if (lhs && detail::is_mul_op(peek_next()))
    {
      auto op = m_lex.next();
      if (auto rhs = multiplicative_expr())
        return m_builder.make_binary(*lhs, *rhs, op);
    }

    return lhs;
  }

  ast::expr* parser::unary_expr() noexcept
  {
    if (!detail::is_unary_op(peek_next()))
      return primary_expr();

    auto op = m_lex.next();
    if (auto exp = primary_expr())
    {
      return m_builder.make_unary(*exp, op);
    }

    return error_expr("Expected expression"sv);
  }

  ast::expr* parser::paren_expr() noexcept
  {
    if (!detail::is_open_paren(peek_next()))
      return error_expr("Unexpected token"sv);

    m_lex.next();

    auto intExpr = expr();
    if (!intExpr)
      return error_expr("Expected expression"sv);

    if (!detail::is_close_paren(peek_next()))
      return error_expr("Expected ')'"sv);

    m_lex.next();
    return m_builder.make_paren(*intExpr);
  }

  ast::expr* parser::primary_expr() noexcept
  {
    auto&& next = peek_next();
    if (next.is_literal())
    {
      return m_builder.make_literal(m_lex.next());
    }

    if (next.is_identifier())
    {
      return m_builder.make_id(m_lex.next());
    }

    return paren_expr();
  }
}