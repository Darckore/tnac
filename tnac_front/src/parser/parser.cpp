#include "parser/parser.hpp"
#include "sema/sema.hpp"
#include "common/feedback.hpp"
#include "common/diag.hpp"
#include "src_mgr/source_file.hpp"

namespace tnac::detail
{
  class op_precedence final
  {
  public:
    enum class prec : std::uint8_t
    {
      LogicalOr,
      LogicalAnd,
      Equality,
      Relational,
      BitOr,
      BitXor,
      BitAnd,
      Additive,
      Multiplicative,
      Power,
      Unary
    };
    using enum prec;

  private:
    inline static constexpr std::array precOrder{
      LogicalAnd,
      Equality,
      Relational,
      BitOr,
      BitXor,
      BitAnd,
      Additive,
      Multiplicative,
      Power,
      Unary
    };


  public:
    CLASS_SPECIALS_ALL(op_precedence);

    constexpr op_precedence(prec cur) noexcept :
      m_cur{ cur }
    {}

    constexpr bool operator==(const op_precedence&) const noexcept = default;

    constexpr prec next() noexcept
    {
      using idx_t = decltype(precOrder)::size_type;
      const auto idx = static_cast<idx_t>(m_cur);
      return precOrder[idx];
    }

    constexpr auto operator*() const noexcept
    {
      return m_cur;
    }

  private:
    prec m_cur{};
  };

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
    auto is_dot(const token& tok) noexcept
    {
      return tok.is(token::Dot);
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

    auto is_entry(const token& tok) noexcept
    {
      return tok.is(token::KwEntry);
    }

    auto is_import(const token& tok) noexcept
    {
      return tok.is(token::KwImport);
    }

    auto is_expr_starter(const token& tok) noexcept
    {
      return tok.is_literal() ||
        tok.is_identifier() ||
        is_type_keyword(tok) ||
        is_open_paren(tok) ||
        is_open_bracket(tok) ||
        is_open_curly(tok) ||
        is_pipe(tok) ||
        is_unary_op(tok);
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

namespace tnac
{
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

  class parser::scope_guard final
  {
  public:
    CLASS_SPECIALS_NONE(scope_guard);

    explicit scope_guard(parser& p, semantics::scope_kind kind) noexcept :
      m_parser{ p }
    {
      m_parser.new_scope(kind);
    }

    ~scope_guard() noexcept
    {
      m_parser.end_scope();
    }

  private:
    parser& m_parser;
  };
}

namespace tnac
{
  // Special members

  parser::~parser() noexcept = default;

  parser::parser(ast::builder& builder, sema& sema, feedback* fb /*= nullptr*/) noexcept :
    parser{ builder, sema, fb, {} }
  {}

  parser::parser(ast::builder& builder, sema& sema, feedback* fb, root_ptr root) noexcept :
    m_builder{ builder },
    m_sema{ sema },
    m_root{ root },
    m_feedback{ fb }
  {}


  // Public members

  parser::pointer parser::operator()(string_t str) noexcept
  {
    return program(str, src::location::dummy());
  }

  parser::pointer parser::operator()(string_t str, loc_t& srcLoc) noexcept
  {
    m_lex.attach_loc(srcLoc);
    auto res = program(str, srcLoc);
    srcLoc.add_line();
    m_lex.detach_loc();
    return res;
  }

  parser::pointer parser::operator()(src::file& input) noexcept
  {
    if (auto parsed = input.parsed_ast())
      return parsed;

    auto loc = input.make_location();
    auto inStr = input.get_contents();
    if (!inStr)
    {
      if (m_feedback)
        m_feedback->error(diag::file_load_failure(input.path(), inStr.error().message()));

      return m_root;
    }

    auto lastWD = fsys::current_path();
    fsys::current_path(input.directory());
    auto parseRes = operator()(*inStr, loc);
    fsys::current_path(lastWD);
    input.attach_ast(*m_curModule);
    m_curModule = {};
    end_scope();
    return parseRes;
  }

  parser::const_root_ptr parser::root() const noexcept
  {
    return m_root;
  }
  parser::root_ptr parser::root() noexcept
  {
    return FROM_CONST(root);
  }

  parser parser::branch() const noexcept
  {
    return { m_builder, m_sema, m_feedback, m_root };
  }


  // Private members(Semantics)

  void parser::new_scope(semantics::scope_kind kind) noexcept
  {
    m_sema.open_scope(kind);
  }
  void parser::end_scope() noexcept
  {
    m_sema.close_scope();
  }


  // Private members(Commands)

  void parser::command(bool consumeSeparator) noexcept
  {
    auto cmdName = peek_next();
    if (!detail::is_command_name(cmdName))
      return;

    next_tok();
    auto argList = command_args(consumeSeparator);
    ast::command cmd{ cmdName, std::move(argList) };
    if(m_feedback)
      m_feedback->command(std::move(cmd));
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


  // Private members(Parsing)

  void parser::start_module(loc_t& loc) noexcept
  {
    init_root();
    if (m_curModule)
      return;

    auto locw = loc.record();
    if (loc.is_dummy())
      m_curModule = m_builder.get_default_module(locw);
    else
      m_curModule = m_builder.make_module(loc.file().stem().string(), locw);

    m_root->append(*m_curModule);
    new_scope(semantics::scope_kind::Module);
    m_sema.visit_module_def(*m_curModule);
    import_seq();
    entry();
  }

  void parser::init_root() noexcept
  {
    if (!m_root)
    {
      m_root = m_builder.make_root();
      new_scope(semantics::scope_kind::Global);
    }
  }

  const token& parser::peek_next() noexcept
  {
    return m_lex.peek();
  }

  token parser::next_tok() noexcept
  {
    m_lastConsumed = m_lex.next();
    return *m_lastConsumed;
  }

  const token& parser::last_tok(const token& dummy) noexcept
  {
    if (!m_lastConsumed)
      return dummy;

    return *m_lastConsumed;
  }

  ast::expr* parser::error_expr(const token& pos, string_t msg, err_pos at) noexcept
  {
    auto errPos = at == err_pos::Current ? pos : last_tok(pos).get_after();
    auto errExpr = m_builder.make_error(errPos, msg);

    if (m_feedback)
      m_feedback->parse_error(*errExpr);

    return errExpr;
  }

  parser::pointer parser::program(string_t input, loc_t& loc) noexcept
  {
    m_lastConsumed.reset();
    m_lex(input);
    start_module(loc);

    pointer res = m_curModule;
    auto eList = expression_list(scope_level::Global);
    if (eList.empty())
    {
      return m_root;
    }

    res = eList.back();
    m_curModule->adopt(std::move(eList));
    return res;
  }

  void parser::entry() noexcept
  {
    if (!detail::is_entry(peek_next()))
      return;

    auto entryKw = next_tok();
    if (auto&& op = peek_next(); !detail::is_open_paren(op))
    {
      error_expr(op, diag::expected('('), err_pos::Current);
      return;
    }

    next_tok();
    auto params = formal_params();
    if (auto&& cp = peek_next(); !detail::is_close_paren(cp))
    {
      auto opt = error_expr(cp, diag::expected(')'), err_pos::Last);
      params.push_back(m_builder.make_param_decl(cp, opt));
    }
    else
    {
      next_tok();
    }

    m_sema.visit_module_entry(*m_curModule, std::move(params), entryKw.at());
  }

  void parser::import_seq() noexcept
  {
    while(auto newImport = import_dir())
      m_curModule->add_import(*newImport);
  }

  ast::import_dir* parser::import_dir() noexcept
  {
    if (!detail::is_import(peek_next()))
      return {};

    auto importKw = next_tok();
    
    import_name name;
    auto depth = 0u;
    SCOPE_GUARD(while (depth)
    {
      --depth;
      end_scope();
    });

    fsys::path loadPath{};
    while (peek_next().is_identifier())
    {
      auto id = next_tok();
      auto idName = id.value();
      loadPath /= idName;

      if (detail::is_dot(peek_next()))
      {
        next_tok();
        auto&& sym = m_sema.visit_import_component(id);
        ++depth;
        name.emplace_back(m_builder.make_id(id, sym));
      }
      else
      {
        if (!m_feedback || !m_feedback->load_file(std::move(loadPath)))
        {
          m_curModule->adopt({ error_expr(id, diag::import_failed(idName), err_pos::Current) });
          return {};
        }

        auto moduleSym = utils::try_cast<semantics::module_sym>(m_sema.find(idName, true));
        if (!moduleSym)
        {
          m_curModule->adopt({ error_expr(id, diag::import_failed(idName), err_pos::Current) });
          return {};
        }

        name.emplace_back(m_builder.make_id(id, *moduleSym));
        break;
      }

      if (auto&& next = peek_next(); !next.is_identifier())
      {
        m_curModule->adopt({ error_expr(next, diag::expected_id(), err_pos::Last) });
        return {};
      }
    }

    if (name.empty())
    {
      m_curModule->adopt({ error_expr(importKw, diag::empty_import(), err_pos::Last) });
      return {};
    }

    return m_builder.make_import(importKw, std::move(name));
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

      // Invalid expressions might be in the middle of other expressions
      if(e->is_valid())
        res.push_back(error_expr(next, diag::expected_expr_sep(), err_pos::Last));
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

    if (!decl->is_valid())
      return error_expr(decl->pos(), diag::invalid_decl(), err_pos::Current);

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
      init = error_expr(op, diag::expected_init(), err_pos::Current);
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
    scope_guard _{ *this, semantics::scope_kind::Function };

    auto params = formal_params();

    if (auto&& cp = peek_next(); !detail::is_close_paren(cp))
    {
      auto opt = error_expr(cp, diag::expected(')'), err_pos::Last);
      params.push_back(m_builder.make_param_decl(cp, opt));
    }
    else
    {
      next_tok();
    }

    auto pos = name;
    if (name.is(token::KwFunction))
    {
      name = token{ m_sema.contrive_name(), token::Identifier };
    }

    auto funcDecl = m_builder.make_func_decl(name, pos, *def, std::move(params));
    if (funcDecl->is_valid())
      m_sema.visit_decl(*funcDecl);

    if (detail::is_semi(peek_next()))
    {
      next_tok();
      return funcDecl;
    }

    auto body = expression_list(scope_level::Nested);
    if (auto&& last = peek_next(); !detail::is_semi(last))
    {
      body.push_back(error_expr(last, diag::expected_func_end(), err_pos::Last));
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
    auto name = peek_next();
    ast::expr* opt{};

    if (!name.is_identifier())
    {
      expr();
      opt = error_expr(name, diag::expected_id(), err_pos::Current);
    }
    else if (auto sym = m_sema.find(name.value(), true))
    {
      next_tok();
      opt = error_expr(name, diag::param_redef(), err_pos::Current);
    }
    else
    {
      next_tok();
      if (auto&& next = peek_next(); !next.is_any(token::Comma, token::ParenClose, token::Semicolon))
      {
        if(!detail::is_expr_starter(next))
          next_tok();

        expr();
        opt = error_expr(name, diag::expr_not_allowed(), err_pos::Current);
      }
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

      if (!paramDecl->definition())
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

    if (auto id = utils::try_cast<ast::id_expr>(lhs); !id)
    {
      lhs = error_expr(lhs->pos(), diag::expected_single_id(), err_pos::Current);
    }
    else if(!detail::is_assignable(id->symbol()))
    {
      lhs = error_expr(lhs->pos(), diag::expected_assignable(), err_pos::Current);
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
      elements.push_back(error_expr(peek_next(), diag::expected(']'), err_pos::Last));
    else
      next_tok();

    return m_builder.make_array(ob, std::move(elements));
  }

  ast::expr* parser::paren_expr() noexcept
  {
    auto op = next_tok();

    VALUE_GUARD(m_terminateAt, tok_kind::Eol);
    auto intExpr = expr();

    if (!detail::is_close_paren(peek_next()))
      return error_expr(peek_next(), diag::expected(')'), err_pos::Last);

    next_tok();
    return m_builder.make_paren(*intExpr, op);
  }

  ast::expr* parser::abs_expr() noexcept
  {
    auto op = next_tok();
    
    VALUE_GUARD(m_terminateAt, tok_kind::Pipe);
    auto intExpr = expr();

    if (!detail::is_pipe(peek_next()))
      return error_expr(peek_next(), diag::expected('|'), err_pos::Last);

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
        return error_expr(next_tok(), diag::undef_id(), err_pos::Current);

      return m_builder.make_id(next_tok(), *sym);
    }

    auto err = next_tok();
    return error_expr(err, diag::expected_expr(), err_pos::Current);
  }

  ast::expr* parser::anonimous_function() noexcept
  {
    UTILS_ASSERT(peek_next().is(token::KwFunction));
    auto kw = next_tok();
    auto funcDecl = func_decl(kw);
    if (!funcDecl->is_valid())
      return error_expr(kw, diag::invalid_lambda(), err_pos::Current);

    return m_builder.make_decl_expr(*funcDecl);
  }

  ast::expr* parser::typed_expr() noexcept
  {
    auto kw = next_tok();

    if (!detail::is_open_paren(peek_next()))
      return error_expr(peek_next(), diag::expected_args(), err_pos::Last);

    next_tok();
    auto args = arg_list(token::ParenClose);

    if (!detail::is_close_paren(peek_next()))
      return error_expr(peek_next(), diag::expected(')'), err_pos::Last);

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
        return error_expr(peek_next(), diag::expected(')'), err_pos::Last);

      next_tok();
      res = m_builder.make_call(*res, std::move(args));
    }

    return res;
  }

  ast::expr* parser::cond_expr() noexcept
  {
    auto scope = m_builder.make_scope({});
    scope_guard _{ *this, semantics::scope_kind::Block };

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

    if(auto&& next = peek_next(); detail::is_close_curly(next))
      return error_expr(peek_next(), diag::empty_cond(), err_pos::Current);
    
    auto c = expr();

    if (!detail::is_close_curly(peek_next()))
      return error_expr(peek_next(), diag::expected('}'), err_pos::Last);

    next_tok();
    return c;
  }

  ast::expr* parser::cond_short(ast::expr& condExpr, ast::scope& scope) noexcept
  {
    if (!detail::is_open_curly(peek_next()))
      return error_expr(peek_next(), diag::expected('{'), err_pos::Current);

    next_tok();
    ast::expr* onTrue{};
    ast::expr* onFalse{};
    
    if (auto&& next = peek_next(); !detail::is_comma(next) &&
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
        auto err = peek_next();
        expr();
        onFalse = error_expr(err, diag::expected(','), err_pos::Current);
      }
    }

    if (auto&& next = peek_next(); !detail::is_close_curly(next))
      return error_expr(next, diag::expected('}'), err_pos::Last);

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
      patterns.push_back(error_expr(peek_next(), diag::expected_cond_end(), err_pos::Last));

    next_tok();
    scope.adopt(std::move(patterns));

    return m_builder.make_conditional(condExpr, scope);
  }

  ast::expr* parser::cond_pattern() noexcept
  {
    auto body = m_builder.make_scope({});
    scope_guard _{ *this, semantics::scope_kind::Block };
    auto matcher = cond_matcher();

    if (!detail::is_semi(peek_next()))
    {
      auto exprList = expression_list(scope_level::Nested);
      if (!detail::is_semi(peek_next()))
      {
        exprList.push_back(error_expr(next_tok(), diag::expected_pattern_end(), err_pos::Last));
      }
      body->adopt(std::move(exprList));
    }

    next_tok();
    return m_builder.make_pattern(*matcher, *body);
  }

  ast::expr* parser::cond_matcher() noexcept
  {
    if (!detail::is_open_curly(peek_next()))
      return error_expr(peek_next(), diag::expected('{'), err_pos::Current);

    auto patternPos = next_tok();
    ast::expr* checked{};

    if (detail::is_pattern_matcher(peek_next()))
    {
      patternPos = next_tok();
      if (detail::is_close_curly(peek_next()))
        return error_expr(next_tok(), diag::expected_expr(), err_pos::Current);
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
        return error_expr(peek_next(), diag::expected('}'), err_pos::Last);

      if (detail::is_open_curly(patternPos))
        patternPos = checked->pos();
    }

    next_tok();
    if (!detail::is_arrow(peek_next()))
      return error_expr(peek_next(), diag::expected_matcher_def(), err_pos::Last);

    next_tok();
    return m_builder.make_matcher(patternPos, checked);
  }

}