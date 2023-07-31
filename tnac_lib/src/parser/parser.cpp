#include "parser/parser.hpp"
#include "sema/sema.hpp"
#include "sema/symbol.hpp"

namespace tnac
{
  namespace detail
  {
    namespace
    {
      auto is_unary_op(const token& tok) noexcept
      {
        return tok.is_any(token::Plus, token::Minus, token::Tilde,
          token::Exclamation, token::Question);
      }
      auto is_add_op(const token& tok) noexcept
      {
        return tok.is_any(token::Plus, token::Minus);
      }
      auto is_mul_op(const token& tok) noexcept
      {
        return tok.is_any(token::Asterisk, token::Slash, token::Percent);
      }
      auto is_pow_op(const token& tok) noexcept
      {
        return tok.is_any(token::Pow, token::Root);
      }
      auto is_assign(const token& tok) noexcept
      {
        return tok.is_any(token::Assign);
      }
      auto is_init(const token& tok) noexcept
      {
        return tok.is(token::Assign);
      }
      auto is_relational(const token& tok) noexcept
      {
        return tok.is_any(token::Less, token::LessEq,
          token::Greater, token::GreaterEq);
      }
      auto is_eq_comparison(const token& tok) noexcept
      {
        return tok.is_any(token::Eq, token::NotEq);
      }
      auto is_pattern_matcher(const token& tok) noexcept
      {
        return is_eq_comparison(tok) || is_relational(tok);
      }
      auto is_pattern_unary(const token& tok) noexcept
      {
        return tok.is_any(token::Exclamation, token::Question);
      }
      auto is_logical(const token& tok) noexcept
      {
        return tok.is_any(token::LogAnd, token::LogOr);
      }

      auto match(op_precedence prec, const token& tok) noexcept
      {
        using enum op_precedence::prec;
        switch (*prec)
        {
        case LogicalOr:      return tok.is(token::LogOr);
        case LogicalAnd:     return tok.is(token::LogAnd);
        case Equality:       return is_eq_comparison(tok);
        case Relational:     return is_relational(tok);
        case BitOr:          return tok.is(token::Pipe);
        case BitXor:         return tok.is(token::Hat);
        case BitAnd:         return tok.is(token::Amp);
        case Additive:       return is_add_op(tok);
        case Multiplicative: return is_mul_op(tok);
        case Power:          return is_pow_op(tok);
        case Unary:          return is_unary_op(tok);

        default: return false;
        }
      }

      auto is_open_paren(const token& tok) noexcept
      {
        return tok.is(token::ParenOpen);
      }
      auto is_close_paren(const token& tok) noexcept
      {
        return tok.is(token::ParenClose);
      }
      auto is_open_curly(const token& tok) noexcept
      {
        return tok.is(token::CurlyOpen);
      }
      auto is_close_curly(const token& tok) noexcept
      {
        return tok.is(token::CurlyClose);
      }
      auto is_open_bracket(const token& tok) noexcept
      {
        return tok.is(token::BracketOpen);
      }
      auto is_close_bracket(const token& tok) noexcept
      {
        return tok.is(token::BracketClose);
      }
      auto is_comma(const token& tok) noexcept
      {
        return tok.is(token::Comma);
      }
      auto is_semi(const token& tok) noexcept
      {
        return tok.is(token::Semicolon);
      }
      auto is_arrow(const token& tok) noexcept
      {
        return tok.is(token::Arrow);
      }
      auto is_pipe(const token& tok) noexcept
      {
        return tok.is(token::Pipe);
      }

      auto is_expression_separator(const token& tok) noexcept
      {
        return tok.is(token::ExprSep);
      }

      auto is_command_name(const token& tok) noexcept
      {
        return tok.is(token::Command);
      }

      auto is_type_keyword(const token& tok) noexcept
      {
        using enum tok_kind;
        return tok.is_any(KwComplex, KwFraction, KwInt, KwFloat, KwBool);
      }

      auto is_error_expr(const ast::expr& expr) noexcept
      {
        return expr.is(ast::node_kind::Error);
      }

      auto is_assignable(const semantics::symbol& sym) noexcept
      {
        using enum semantics::sym_kind;
        return sym.is_any(Variable, Parameter);
      }
    }
  }

  bool has_implicit_separator(const ast::node& expr) noexcept
  {
    using kind = ast::node_kind;
    if (auto binary = utils::try_cast<kind::Binary>(&expr))
      return has_implicit_separator(binary->right());

    if (auto unary = utils::try_cast<kind::Unary>(&expr))
      return has_implicit_separator(unary->operand());

    if (!expr.is(kind::Decl))
      return expr.is_any(kind::Cond, kind::Pattern);

    auto&& decl = utils::cast<ast::decl_expr>(expr).declarator();
    if (decl.is(kind::FuncDecl))
      return true;
    
    auto def = decl.definition();
    return def && has_implicit_separator(*def);
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

  parser::pointer parser::operator()(string_t str, loc& srcLoc) noexcept
  {
    m_lex.attach_loc(srcLoc);
    auto res = operator()(str);
    m_lex.detach_loc();
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

      res.emplace_back(next_tok());
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
    skip_to(tok_kind::ExprSep);
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

      auto e = ret_expr();
      UTILS_ASSERT(static_cast<bool>(e));
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

      if (has_implicit_separator(*e))
        continue;

      // postpone to the caller
      if (scopeLvl == scope_level::Nested)
        break;

      // invalid expressions might be in the middle of other expressions
      if(e->is_valid())
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

  ast::expr* parser::ret_expr() noexcept
  {
    if (peek_next().is(token::KwRet))
    {
      auto pos = next_tok();
      auto retVal = expr();
      return m_builder.make_ret(*retVal, pos);
    }

    return expr();
  }

  ast::decl* parser::declarator() noexcept
  {
    auto&& next = peek_next();
    if (!next.is_identifier())
      return {};

    if (m_sema.find(next.value()))
      return {};

    return var_decl(next_tok());
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
      next_tok();
      init = error_expr(op, "Expected initialisation"sv);
      expr();
    }

    auto varDecl = m_builder.make_var_decl(name, *init);
    m_sema.visit_decl(*varDecl);
    return varDecl;
  }

  ast::decl* parser::func_decl(token name) noexcept
  {
    UTILS_ASSERT(detail::is_open_paren(peek_next()));
    next_tok();

    auto def = m_builder.make_scope({});
    scope_guard _{ *this, def };

    auto params = formal_params();

    if (!detail::is_close_paren(peek_next()))
      return {};

    for (auto param : params)
    {
      if (param->definition())
        return {};
    }

    auto pos = name;
    if (name.is(token::KwFunction))
    {
      name = m_sema.contrive_name();
    }

    auto funcDecl = m_builder.make_func_decl(name, pos, *def, std::move(params));
    m_sema.visit_decl(*funcDecl);

    next_tok();

    if (detail::is_semi(peek_next()))
    {
      next_tok();
      return funcDecl;
    }

    auto body = expression_list(scope_level::Nested);
    if (auto&& last = peek_next(); !detail::is_semi(last))
    {
      body.push_back(error_expr(last, "Expected ';' at function definition end"sv));
    }
    else
    {
      next_tok();
    }

    def->adopt(std::move(body));
    return funcDecl;
  }

  ast::param_decl* parser::param_decl() noexcept
  {
    auto name = next_tok();
    ast::expr* opt{};

    if (!name.is_identifier())
    {
      opt = error_expr(name, "Expected identifier"sv);
    }
    else if (auto sym = m_sema.find(name.value(), true))
    {
      opt = error_expr(name, "Function parameter redifinition"sv);
    }

    return m_builder.make_param_decl(name, opt);
  }

  parser::param_list parser::formal_params() noexcept
  {
    param_list res;
    
    for (;;)
    {
      if (detail::is_close_paren(peek_next()))
        break;

      auto paramDecl = param_decl();
      res.push_back(paramDecl);

      if (paramDecl->definition())
        skip_to(tok_kind::Comma, tok_kind::ParenClose);
      else
        m_sema.visit_decl(*paramDecl);

      auto&& next = peek_next();

      if (detail::is_comma(next))
      {
        next_tok();
        continue;
      }

      if (!detail::is_comma(next))
        break;
    }

    return res;
  }

  ast::expr* parser::assign_expr() noexcept
  {
    auto lhs = binary_expr();
    
    if (!detail::is_assign(peek_next()))
      return lhs;

    if (!lhs->is(ast::node::Identifier))
    {
      auto err = error_expr(lhs->pos(), "Expected a single identifier"sv, true);
      return err;
    }

    if (auto&& sym = utils::cast<ast::id_expr>(*lhs).symbol();
              !detail::is_assignable(sym))
    {
      auto err = error_expr(lhs->pos(), "Expected an assignable object"sv);
      lhs = err;
    }

    auto op = next_tok();
    auto rhs = assign_expr();
    return m_builder.make_assign(*lhs, *rhs, op);
  }

  ast::expr* parser::binary_expr() noexcept
  {
    return binary_expr(prec::LogicalOr);
  }

  ast::expr* parser::binary_expr(prec precedence) noexcept
  {
    detail::op_precedence pr{ precedence };
    auto res = expr_by_prec(precedence);

    for (;;)
    {
      auto&& next = peek_next();
      
      if (next.is(m_terminateAt))
        break;

      if (!detail::match(pr, next))
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
      return call_expr();

    auto op = next_tok();
    auto exp = unary_expr();
    return m_builder.make_unary(*exp, op);
  }

  ast::expr* parser::array_expr() noexcept
  {
    auto ob = next_tok();
    auto elements = arg_list(token::BracketClose);

    if (!detail::is_close_bracket(peek_next()))
      return error_expr(next_tok(), "Expected ']'"sv, true);

    next_tok();  
    return m_builder.make_array(ob, std::move(elements));
  }

  ast::expr* parser::paren_expr() noexcept
  {
    auto op = next_tok();

    VALUE_GUARD(m_terminateAt, tok_kind::Eol);
    auto intExpr = expr();

    if (!detail::is_close_paren(peek_next()))
      return error_expr(peek_next(), "Expected ')'"sv);

    next_tok();
    return m_builder.make_paren(*intExpr, op);
  }

  ast::expr* parser::abs_expr() noexcept
  {
    auto op = next_tok();
    
    VALUE_GUARD(m_terminateAt, tok_kind::Pipe);
    auto intExpr = expr();

    if (!detail::is_pipe(peek_next()))
      return error_expr(peek_next(), "Expected '|'"sv);

    next_tok();
    return m_builder.make_abs(*intExpr, op);
  }

  ast::expr* parser::primary_expr() noexcept
  {
    auto&& next = peek_next();

    if (next.is_literal())
      return m_builder.make_literal(next_tok());

    if (detail::is_type_keyword(next))
      return typed_expr();

    if (next.is(token::KwFunction))
      return anonimous_function();

    if (next.is(token::KwResult))
      return m_builder.make_result(next_tok());

    if (detail::is_open_bracket(next))
      return array_expr();

    if (detail::is_pipe(next))
      return abs_expr();

    if (detail::is_open_paren(next))
      return paren_expr();

    if (detail::is_open_curly(next))
      return cond_expr();

    if (next.is(token::Identifier))
    {
      auto sym = m_sema.find(next.value());

      if (!sym)
        return error_expr(next_tok(), "Undefined identifier"sv);

      auto name = next_tok();
      return m_builder.make_id(name, *sym);
    }

    return error_expr(next, "Expected expression"sv, true);
  }

  ast::expr* parser::anonimous_function() noexcept
  {
    UTILS_ASSERT(peek_next().is(token::KwFunction));
    auto kw = next_tok();
    auto funcDecl = func_decl(kw);
    if (!funcDecl)
      return error_expr(kw, "Invalid anonimous function definition"sv);

    return m_builder.make_decl_expr(*funcDecl);
  }

  ast::expr* parser::typed_expr() noexcept
  {
    auto kw = next_tok();

    if (!detail::is_open_paren(peek_next()))
      return error_expr(next_tok(), "Expected argument list"sv);

    next_tok();
    auto args = arg_list(token::ParenClose);

    if (!detail::is_close_paren(peek_next()))
      return error_expr(next_tok(), "Expected ')'"sv);

    next_tok();
    return m_builder.make_typed(kw, std::move(args));
  }

  parser::expr_list parser::arg_list(tok_kind closing) noexcept
  {
    expr_list res;

    while (!peek_next().is_eol())
    {
      if (peek_next().is(closing))
        break;

      auto e = expr();
      res.push_back(e);

      if (!detail::is_comma(peek_next()))
        break;

      next_tok();
    }

    return res;
  }

  ast::expr* parser::call_expr() noexcept
  {
    auto res = primary_expr();
    
    while (detail::is_open_paren(peek_next()))
    {
      next_tok();
      auto args = arg_list(token::ParenClose);

      if (!detail::is_close_paren(peek_next()))
        return error_expr(next_tok(), "Expected ')'"sv, true);

      next_tok();
      res = m_builder.make_call(*res, std::move(args));
    }

    return res;
  }

  ast::expr* parser::cond_expr() noexcept
  {
    auto scope = m_builder.make_scope({});
    scope_guard _{ *this, scope };

    auto condExpr = cond();
    if (detail::is_arrow(peek_next()))
    {
      next_tok();
      return cond_short(*condExpr, *scope);
    }

    return cond_body(*condExpr, *scope);
  }

  ast::expr* parser::cond() noexcept
  {
    UTILS_ASSERT(detail::is_open_curly(peek_next()));
    next_tok();

    auto c = expr();

    if (!detail::is_close_curly(peek_next()))
      return error_expr(next_tok(), "Expected '}'"sv);

    next_tok();
    return c;
  }

  ast::expr* parser::cond_short(ast::expr& condExpr, ast::scope& scope) noexcept
  {
    if (!detail::is_open_curly(peek_next()))
      return error_expr(next_tok(), "Expected '{'"sv, true);

    next_tok();
    ast::expr* onTrue{};
    ast::expr* onFalse{};
    
    if (auto&& next = peek_next(); !detail::is_comma(peek_next()) &&
                                   !detail::is_close_curly(next))
    {
      onTrue = expr();
    }

    if (!detail::is_close_curly(peek_next()))
    {
      if (detail::is_comma(peek_next()))
      {
        next_tok();
        onFalse = expr();
      }
      else
      {
        onFalse = error_expr(next_tok(), "Expected ','"sv);
        skip_to(token::ExprSep, token::CurlyClose, token::Eol);
      }
    }

    if (!detail::is_close_curly(peek_next()))
      return error_expr(next_tok(), "Expected '}'"sv, true);

    next_tok();
    return m_builder.make_short_cond(condExpr, onTrue, onFalse, scope);
  }

  ast::expr* parser::cond_body(ast::expr& condExpr, ast::scope& scope) noexcept
  {
    expr_list patterns;
    while (!peek_next().is_any(token::Eol, token::Semicolon))
    {
      patterns.push_back(cond_pattern());
    }

    if (!detail::is_semi(peek_next()))
      patterns.push_back(error_expr(peek_next(), "Expected ';' at the end of conditional"sv));

    next_tok();
    scope.adopt(std::move(patterns));

    return m_builder.make_conditional(condExpr, scope);
  }

  ast::expr* parser::cond_pattern() noexcept
  {
    auto body = m_builder.make_scope({});
    scope_guard _{ *this, body };
    auto matcher = cond_matcher();

    if (!detail::is_semi(peek_next()))
    {
      auto exprList = expression_list(scope_level::Nested);
      if (!detail::is_semi(peek_next()))
      {
        exprList.push_back(error_expr(next_tok(), "Expected ';' at the end of pattern body"sv));
      }
      body->adopt(std::move(exprList));
    }

    next_tok();
    return m_builder.make_pattern(*matcher, *body);
  }

  ast::expr* parser::cond_matcher() noexcept
  {
    if (!detail::is_open_curly(peek_next()))
      return error_expr(next_tok(), "Expected '{'"sv);

    auto patternPos = next_tok();
    ast::expr* checked{};

    if (detail::is_pattern_matcher(peek_next()))
    {
      patternPos = next_tok();
      if (detail::is_close_curly(peek_next()))
        return error_expr(next_tok(), "Expected expression"sv);
    }
    else if (detail::is_pattern_unary(peek_next()))
    {
      patternPos = next_tok();
    }

    if (!detail::is_close_curly(peek_next()))
    {
      checked = expr();
      if (detail::is_pattern_unary(patternPos))
        checked = m_builder.make_unary(*checked, patternPos);

      if (!detail::is_close_curly(peek_next()))
        return error_expr(peek_next(), "Expected '}'"sv);

      if (detail::is_open_curly(patternPos))
        patternPos = checked->pos();
    }

    next_tok();
    if (!detail::is_arrow(peek_next()))
      return error_expr(next_tok(), "Expected '->' after condition matcher"sv);

    next_tok();
    return m_builder.make_matcher(patternPos, checked);
  }

}