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
      constexpr auto is_assign(const token& tok) noexcept
      {
        return tok.is_any(token::Assign);
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

      auto is_error_expr(ast::expr* expr) noexcept
      {
        return expr->what() == ast::node_kind::Error;
      }
    }
  }

  // Special members

  parser::~parser() noexcept = default;

  parser::parser(ast::builder& builder, sema& sema) noexcept :
    m_builder{ builder },
    m_sema{ sema }
  {}


  // Public members

  parser::pointer parser::operator()(string_t str) noexcept
  {
    m_lex(str);

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
    return FROM_CONST(root);
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
    return m_builder.make_error(pos, msg);
  }

  parser::expr_list parser::expression_list() noexcept
  {
    expr_list res;

    while (!peek_next().is_eol())
    {
      auto e = expr();
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
      to_expr_end();
    }

    return res;
  }

  ast::expr* parser::expr() noexcept
  {
    return decl_expr();
  }

  ast::expr* parser::decl_expr() noexcept
  {
    // todo: symbol table
    return assign_expr();
  }

  ast::expr* parser::assign_expr() noexcept
  {
    auto lhs = additive_expr();
    
    if (!lhs->is(ast::node::Identifier))
    {
      if (!detail::is_assign(peek_next()))
        return lhs;

      auto err = m_builder.make_error(lhs->pos(), "Expected a single identifier");
      to_expr_end();
      return err;
    }

    auto op = m_lex.next();
    auto rhs = assign_expr();
    return m_builder.make_assign(*lhs, *rhs, op);
  }

  ast::expr* parser::additive_expr() noexcept
  {
    auto lhs = multiplicative_expr();
    return additive_rhs(lhs);
  }

  ast::expr* parser::additive_rhs(ast::expr* lhs) noexcept
  {
    if(!detail::is_add_op(peek_next()))
      return lhs;

    auto op = m_lex.next();
    auto rhs = multiplicative_expr();
    auto lhs_next = m_builder.make_binary(*lhs, *rhs, op);

    return additive_rhs(lhs_next);
  }

  ast::expr* parser::multiplicative_expr() noexcept
  {
    auto lhs = unary_expr();
    return multiplicative_rhs(lhs);
  }

  ast::expr* parser::multiplicative_rhs(ast::expr* lhs) noexcept
  {
    if (!detail::is_mul_op(peek_next()))
      return lhs;

    auto op = m_lex.next();
    auto rhs = unary_expr();
    auto lhs_next = m_builder.make_binary(*lhs, *rhs, op);

    return multiplicative_rhs(lhs_next);
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

    auto op = m_lex.next();

    auto intExpr = expr();
    if (detail::is_error_expr(intExpr))
      return error_expr("Expected expression"sv);

    if (!detail::is_close_paren(peek_next()))
      return error_expr("Expected ')'"sv);

    m_lex.next();
    return m_builder.make_paren(*intExpr, op);
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