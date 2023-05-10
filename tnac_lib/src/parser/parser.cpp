#include "parser/parser.hpp"
#include "sema/sema.hpp"
#include "sema/symbol.hpp"

namespace tnac
{
  namespace detail
  {
    namespace
    {
      constexpr auto is_unary_op(const token& tok) noexcept
      {
        return tok.is_any(token::Plus, token::Minus, token::Tilde);
      }
      constexpr auto is_add_op(const token& tok) noexcept
      {
        return tok.is_any(token::Plus, token::Minus);
      }
      constexpr auto is_mul_op(const token& tok) noexcept
      {
        return tok.is_any(token::Asterisk, token::Slash, token::Percent);
      }
      constexpr auto is_assign(const token& tok) noexcept
      {
        return tok.is_any(token::Assign);
      }
      constexpr auto is_init(const token& tok) noexcept
      {
        return tok.is(token::Assign);
      }

      constexpr auto match(op_precedence prec, const token& tok) noexcept
      {
        using enum op_precedence::prec;
        switch (*prec)
        {
        case BitOr:
          return tok.is(token::Pipe);

        case BitXor:
          return tok.is(token::Hat);

        case BitAnd:
          return tok.is(token::Amp);

        case Additive:
          return is_add_op(tok);

        case Multiplicative:
          return is_mul_op(tok);

        case Unary:
          return is_unary_op(tok);

        default:
          return false;
        }
      }

      constexpr auto is_open_paren(const token& tok) noexcept
      {
        return tok.is(token::ParenOpen);
      }
      constexpr auto is_close_paren(const token& tok) noexcept
      {
        return tok.is(token::ParenClose);
      }
      constexpr auto is_comma(const token& tok) noexcept
      {
        return tok.is(token::Comma);
      }
      constexpr auto is_semi(const token& tok) noexcept
      {
        return tok.is(token::Semicolon);
      }

      constexpr auto is_expression_separator(const token& tok) noexcept
      {
        return tok.is(token::ExprSep);
      }

      constexpr auto is_command_name(const token& tok) noexcept
      {
        return tok.is(token::Command);
      }

      constexpr auto is_type_keyword(const token& tok) noexcept
      {
        return tok.is_any(token::KwComplex, token::KwFraction);
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

    if (!m_root)
    {
      m_root = m_builder.make_scope({});
      new_scope(m_root);
    }

    pointer res = m_root;
    auto eList = expression_list(scope_level::Global);
    if (!eList.empty())
    {
      res = eList.back();
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

  /// Semantics

  void parser::new_scope(root_ptr node) noexcept
  {
    m_sema.open_scope(*node);
  }
  void parser::end_scope() noexcept
  {
    m_sema.close_scope();
  }

  /// Commands

  void parser::command(bool consumeSeparator) noexcept
  {
    auto cmdName = peek_next();
    if (!detail::is_command_name(cmdName))
      return;

    if (!m_cmdHandler)
    {
      to_expr_end();
      
      if(consumeSeparator)
        next_tok();

      return;
    }

    next_tok();
    auto argList = command_args(consumeSeparator);
    ast::command cmd{ cmdName, std::move(argList) };
    m_cmdHandler(std::move(cmd));
  }

  ast::command::arg_list parser::command_args(bool consumeSeparator) noexcept
  {
    ast::command::arg_list res{};

    for (;;)
    {
      auto&& next = peek_next();
      
      if (next.is_eol())
        break;

      if (detail::is_expression_separator(next))
      {
        if(consumeSeparator)
          next_tok();

        break;
      }

      res.push_back(next_tok());
    }

    return res;
  }

  /// Parsing

  const token& parser::peek_next() noexcept
  {
    return m_lex.peek();
  }

  token parser::next_tok() noexcept
  {
    return m_lex.next();
  }

  void parser::to_expr_end() noexcept
  {
    for (;;)
    {
      auto next = peek_next();
      if (next.is_eol() || detail::is_expression_separator(next))
        break;

      next_tok();
    }
  }

  ast::expr* parser::error_expr(token pos, string_t msg, bool skipRest /*= false*/) noexcept
  {
    if (skipRest)
      to_expr_end();

    auto errExpr = m_builder.make_error(pos, msg);

    if (m_errHandler)
      m_errHandler(*errExpr);

    return errExpr;
  }

  parser::expr_list parser::expression_list(scope_level scopeLvl) noexcept
  {
    expr_list res;

    while (!peek_next().is_eol())
    {
      command(true);
      if (peek_next().is_eol())
        break;

      auto e = expr();
      res.push_back(e);

      command(false);

      auto&& next = peek_next();
      if (next.is_eol())
        break;

      if (scopeLvl == scope_level::Nested && detail::is_semi(next))
        break;

      if (detail::is_expression_separator(next))
      {
        next_tok();
        continue;
      }

      res.push_back(error_expr(next, "Expected ':' or EOL"sv, true));
    }

    return res;
  }

  ast::expr* parser::expr() noexcept
  {
    return decl_expr();
  }

  ast::expr* parser::decl_expr() noexcept
  {
    auto decl = declarator();
    if(!decl)
      return assign_expr();

    return m_builder.make_decl_expr(*decl);
  }

  ast::decl* parser::declarator() noexcept
  {
    auto&& next = peek_next();
    if (!next.is_identifier())
      return {};

    if (m_sema.find(next.m_value))
      return {};

    auto res = var_decl(next_tok());
    if(res)
      m_sema.visit_decl(*res);
    
    return res;
  }

  ast::decl* parser::var_decl(token name) noexcept
  {
    auto op = peek_next();

    ast::expr* init{};
    if (detail::is_init(op))
    {
      next_tok();
      init = expr();
    }
    else if (detail::is_open_paren(op))
    {
      return func_decl(name);
    }
    else
    {
      init = error_expr(op, "Expected initialisation"sv, true);
    }

    return m_builder.make_var_decl(name, *init);
  }

  ast::decl* parser::func_decl(token name) noexcept
  {
    UTILS_ASSERT(detail::is_open_paren(peek_next()));
    next_tok();

    auto def = m_builder.make_scope({});
    scope_guard _{ *this, def };

    utils::unused(name);
    return {};
  }

  parser::param_list parser::formal_params() noexcept
  {
    return {};
  }

  ast::expr* parser::assign_expr() noexcept
  {
    auto lhs = binary_expr(prec::BitOr);
    
    if (!detail::is_assign(peek_next()))
      return lhs;

    if (!lhs->is(ast::node::Identifier))
    {
      auto err = error_expr(lhs->pos(), "Expected a single identifier"sv, true);
      return err;
    }

    auto op = next_tok();
    auto rhs = assign_expr();
    return m_builder.make_assign(*lhs, *rhs, op);
  }

  ast::expr* parser::binary_expr(prec precedence) noexcept
  {
    detail::op_precedence pr{ precedence };
    auto res = expr_by_prec(precedence);

    for (;;)
    {
      if (!detail::match(pr, peek_next()))
        break;

      auto op = next_tok();
      auto rhs = expr_by_prec(precedence);
      res = m_builder.make_binary(*res, *rhs, op);
    }

    return res;
  }

  ast::expr* parser::expr_by_prec(prec precedence) noexcept
  {
    precedence = precedence.next();

    if (precedence == prec::Unary)
      return unary_expr();

    return binary_expr(precedence);
  }

  ast::expr* parser::unary_expr() noexcept
  {
    if (!detail::is_unary_op(peek_next()))
      return primary_expr();

    auto op = next_tok();
    auto exprFirst = peek_next();
    auto exp = primary_expr();
    return m_builder.make_unary(*exp, op);
  }

  ast::expr* parser::paren_expr() noexcept
  {
    auto op = next_tok();
    auto intExpr = expr();

    if (!detail::is_close_paren(peek_next()))
      return error_expr(next_tok(), "Expected ')'"sv);

    next_tok();
    return m_builder.make_paren(*intExpr, op);
  }

  ast::expr* parser::primary_expr() noexcept
  {
    auto&& next = peek_next();
    if (next.is_literal())
    {
      return m_builder.make_literal(next_tok());
    }

    if (detail::is_type_keyword(next))
    {
      return typed_expr();
    }

    if (next.is(token::KwResult))
    {
      return m_builder.make_result(next_tok());
    }

    if (next.is_identifier())
    {
      return id_expr();
    }

    if (detail::is_open_paren(peek_next()))
    {
      return paren_expr();
    }

    return error_expr(next, "Expected expression"sv, true);
  }

  ast::expr* parser::typed_expr() noexcept
  {
    auto kw = next_tok();

    if (!detail::is_open_paren(peek_next()))
      return error_expr(next_tok(), "Expected parameter list"sv);

    next_tok();
    auto params = arg_list();

    if (!detail::is_close_paren(peek_next()))
      return error_expr(next_tok(), "Expected ')'"sv);

    next_tok();
    return m_builder.make_typed(kw, std::move(params));
  }

  parser::expr_list parser::arg_list() noexcept
  {
    expr_list res;

    while (!peek_next().is_eol())
    {
      if (detail::is_close_paren(peek_next()))
        break;

      auto e = expr();
      res.push_back(e);

      if (!detail::is_comma(peek_next()))
        break;

      next_tok();
    }

    return res;
  }

  ast::expr* parser::id_expr() noexcept
  {
    auto&& next = peek_next();
    auto sym = m_sema.find(next.m_value);
    
    if (!sym)
      return error_expr(next_tok(), "Undefined identifier"sv);

    return m_builder.make_id(next_tok(), *sym);
  }
}