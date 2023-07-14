#include "evaluator/evaluator.hpp"
#include "evaluator/value/value.hpp"
#include "sema/symbol.hpp"

namespace tnac
{
  namespace detail
  {
    namespace
    {
      constexpr auto conv_unary(tok_kind tk) noexcept
      {
        using enum tok_kind;
        using enum eval::val_ops;
        switch (tk)
        {
        case Exclamation: return LogicalNot;
        case Question:    return LogicalIs;
        case Plus:        return UnaryPlus;
        case Minus:       return UnaryNegation;
        case Tilde:       return UnaryBitwiseNot;
          
        default: return InvalidOp;
        }
      }
      constexpr auto conv_binary(tok_kind tk) noexcept
      {
        using enum tok_kind;
        using enum eval::val_ops;
        switch (tk)
        {
        case Plus:     return Addition;
        case Minus:    return Subtraction;
        case Asterisk: return Multiplication;
        case Slash:    return Division;
        case Percent:  return Modulo;

        case Less:      return RelLess;
        case LessEq:    return RelLessEq;
        case Greater:   return RelGr;
        case GreaterEq: return RelGrEq;
        case Eq:        return Equal;
        case NotEq:     return NEqual;

        case Amp:  return BitwiseAnd;
        case Hat:  return BitwiseXor;
        case Pipe: return BitwiseOr;

        case Pow:  return BinaryPow;
        case Root: return BinaryRoot;

        default: return InvalidOp;
        }
      }
      constexpr auto is_logical(tok_kind tk) noexcept
      {
        return utils::eq_any(tk, tok_kind::LogAnd, tok_kind::LogOr);
      }

      inline auto is_direct_scope_child(const ast::node* node) noexcept
      {
        if (!node)
          return false;

        auto parent = node->parent();
        return parent && parent->is(ast::node_kind::Scope);
      }

      //
      // Helper object for instantiations
      //
      template <eval::detail::expr_result T>
      class instance
      {
      public:
        using err_handler_t = evaluator::err_handler_t;
        using arg_list_t    = evaluator::arg_list_t;
        using size_type     = arg_list_t::size_type;
        using visitor       = eval::value_visitor;
        using value_type    = T;
        using type_info     = eval::type_info<value_type>;

        static constexpr auto min = type_info::minArgs;
        static constexpr auto max = type_info::maxArgs;

      public:
        CLASS_SPECIALS_NONE(instance);

        instance(visitor& valVisitor, err_handler_t& onError) noexcept :
          m_visitor{ valVisitor },
          m_errHandler{ onError }
        {}

        void operator()(const ast::typed_expr& expr) noexcept
        {
          if (!check_args(expr))
          {
            for (auto _: expr.args())
            {
              utils::unused(_);
              m_visitor.fetch_next();
            }
            m_visitor.push_value({});
            return;
          }

          instantiate(expr, std::make_index_sequence<max>{});
        }

      private:
        template <typename T, T... Seq>
        void instantiate(const ast::typed_expr& expr, std::integer_sequence<T, Seq...>) noexcept
        {
          auto&& exprArgs = expr.args();
          std::array<eval::temporary, max> args{};
          for (auto idx = max; idx > size_type{}; --idx)
          {
            const auto itemIndex = idx - 1;
            args[itemIndex] = extract(exprArgs, itemIndex);
          }
          m_visitor.instantiate<value_type>(std::move(args[Seq])...);
        }

        void on_error(const token& pos, string_t msg) noexcept
        {
          if (m_errHandler)
            m_errHandler(pos, msg);
        }

        bool check_args(const ast::typed_expr& expr) noexcept
        {
          const auto size = expr.args().size();
          if (utils::in_range(size, min, max))
            return true;

          auto msg = std::format("Expected at least {} and at most {} arguments"sv, min, max);
          on_error(expr.type_name(), msg);

          return false;
        }

        eval::temporary extract(const arg_list_t& args, size_type idx) noexcept
        {
          if (args.size() <= idx)
            return {};

          return m_visitor.fetch_next();
        }

      private:
        visitor& m_visitor;
        err_handler_t& m_errHandler;
      };
    }
  }

  // Special members

  evaluator::evaluator(eval::registry& registry, eval::call_stack& callStack) noexcept :
    m_visitor{ registry },
    m_callStack{ callStack }
  {}

  // Public members

  void evaluator::operator()(ast::node* root) noexcept
  {
    traverse(root);

    // Remove the in-flight temporary value of the previous expression
    // (if we're given a part of the ast rather than an entire scope)
    if (detail::is_direct_scope_child(root))
      exit_child();
  }

  // Expressions

  void evaluator::visit(ast::assign_expr& assign) noexcept
  {
    utils::unused(assign);
    //if (return_path())
    //  return;

    //auto&& left = assign.left();
    //if (auto assignee = utils::try_cast<ast::id_expr>(&left))
    //{
    //  auto&& lhs = assignee->symbol();
    //  eval_assign(lhs, assign.right().value());
    //  assignee->eval_result(lhs.value());
    //}

    //assign.eval_result(left.value());
  }

  void evaluator::visit(ast::binary_expr& binary) noexcept
  {
    if (return_path())
      return;

    const auto opcode = binary.op().m_kind;

    // We've already calculated logical && and || at this point
    if (detail::is_logical(opcode))
      return;

    auto right = m_visitor.fetch_next();
    auto left  = m_visitor.fetch_next();
    const auto opCode = detail::conv_binary(opcode);
    m_visitor.visit_binary(*left, *right, opCode);
  }

  void evaluator::visit(ast::unary_expr& unary) noexcept
  {
    if (return_path())
      return;

    const auto opCode = detail::conv_unary(unary.op().m_kind);
    auto val = m_visitor.fetch_next();
    m_visitor.visit_unary(*val, opCode);
  }

  void evaluator::visit(ast::typed_expr& expr) noexcept
  {
    if (return_path())
      return;

    using enum tok_kind;
    using detail::instance;

    switch (expr.type_name().m_kind)
    {
    case KwComplex:  instance<eval::complex_type>{ m_visitor, m_errHandler }(expr);  break;
    case KwFraction: instance<eval::fraction_type>{ m_visitor, m_errHandler }(expr); break;
    
    default: UTILS_ASSERT(false); break;
    }
  }

  void evaluator::visit(ast::call_expr& expr) noexcept
  {
    utils::unused(expr);
    //if (return_path())
    //  return;

    //auto func = expr.callable().value();
    //if (auto arr = func.try_get<eval::array_type>())
    //{
    //  make_arr_call(*arr, expr);
    //}
    //else
    //{
    //  auto funcType = func.try_get<eval::function_type>();
    //  make_call(funcType, expr);
    //}

    //auto res = m_visitor.last_result(&expr);
    //expr.eval_result(res);
  }

  void evaluator::visit(ast::ret_expr& ret) noexcept
  {
    utils::unused(ret);
    //if (return_path())
    //  return;

    //auto returnedVal = m_visitor.visit_assign(&ret, ret.returned_value().value());
    //ret.eval_result(returnedVal);
    //m_return = true;
  }

  void evaluator::visit(ast::array_expr& arr) noexcept
  {
    utils::unused(arr);
    //if (return_path())
    //  return;

    //auto&& elements = arr.elements();
    //const auto arrSz = elements.size();
    //auto&& newArr = m_visitor.new_array(&arr, arrSz);
    //for (auto elem : elements)
    //{
    //  newArr.emplace_back(elem->value());
    //}
    //arr.eval_result(m_visitor.make_array(&arr, newArr));
  }

  void evaluator::visit(ast::paren_expr& ) noexcept
  {
    if (return_path())
      return;

    auto exprVal = m_visitor.fetch_next();
    m_visitor.push_value(*exprVal);
  }

  void evaluator::visit(ast::abs_expr& ) noexcept
  {
    if (return_path())
      return;

    const auto opcode = eval::val_ops::AbsoluteValue;
    auto operand = m_visitor.fetch_next();
    m_visitor.visit_unary(*operand, opcode);
  }

  void evaluator::visit(ast::lit_expr& lit) noexcept
  {
    if (return_path())
      return;

    eval_token(lit.pos());
  }

  void evaluator::visit(ast::id_expr& id) noexcept
  {
    utils::unused(id);
    //if (return_path())
    //  return;

    //auto&& sym = id.symbol();
    //auto val = sym.value();
    //m_visitor.visit_assign(nullptr, val);
    //id.eval_result(val);
  }

  void evaluator::visit(ast::result_expr& res) noexcept
  {
    utils::unused(res);
    //if (return_path())
    //  return;

    //res.eval_result(m_visitor.last_result(&res));
  }

  // Decls

  void evaluator::visit(ast::decl_expr& expr) noexcept
  {
    utils::unused(expr);
    //if (return_path())
    //  return;

    //auto&& sym = expr.declarator().symbol();
    //auto val = sym.value();
    //m_visitor.visit_assign(nullptr, val);
    //expr.eval_result(val);
  }

  void evaluator::visit(ast::var_decl& decl) noexcept
  {
    utils::unused(decl);
    //if (return_path())
    //  return;

    //eval_assign(decl.symbol(), decl.initialiser().value());
  }

  void evaluator::visit(ast::func_decl& decl) noexcept
  {
    utils::unused(decl);
    //if (return_path())
    //  return;

    //auto sym = utils::try_cast<semantics::function>(&decl.symbol());

    //if (!sym)
    //{
    //  on_error(decl.pos(), "Invalid symbol type"sv);
    //  return;
    //}

    //if (sym->value())
    //  return;

    //make_function(*sym);
  }

  // Previews

  bool evaluator::preview(ast::func_decl& ) noexcept
  {
    return false;
  }

  bool evaluator::preview(ast::binary_expr& expr) noexcept
  {
    if (return_path())
      return false;

    const auto opcode = expr.op().m_kind;
    if (!detail::is_logical(opcode))
      return true;

    auto&& lhs = expr.left();
    traverse(&lhs);

    auto lval = to_bool(*m_visitor.fetch_next());

    bool res{};
    if ((!lval && opcode == tok_kind::LogAnd) ||
        ( lval && opcode == tok_kind::LogOr))
    {
      res = lval;
    }
    else
    {
      auto&& rhs = expr.right();
      traverse(&rhs);
      auto rval  = to_bool(*m_visitor.fetch_next());
      res = (opcode == tok_kind::LogAnd) ? (lval && rval) : (lval || rval);
    }

    m_visitor.visit_bool_literal(res);
    return false;
  }

  bool evaluator::preview(ast::cond_expr& expr) noexcept
  {
    utils::unused(expr);
    //if (return_path())
    //  return false;

    //auto&& cond = expr.cond();
    //base::operator()(&cond);
    //auto condVal = cond.value();

    //ast::pattern* trueBranch{};
    //ast::pattern* defaultBranch{};
    //using eval::val_ops;
    //for (auto child : expr.patterns().children())
    //{
    //  auto&& pattern = utils::cast<ast::pattern>(*child);
    //  auto&& matcher = utils::cast<ast::matcher>(pattern.matcher());
    //  eval::value currentMatch{};
    //  if (matcher.is_default())
    //  {
    //    defaultBranch = &pattern;
    //    continue;
    //  }
    //  else if (matcher.is_unary())
    //  {
    //    const auto opcode = detail::conv_unary(matcher.pos().m_kind);
    //    currentMatch = m_visitor.visit_unary(&matcher, condVal, opcode);
    //  }
    //  else
    //  {
    //    auto&& checkedExpr = matcher.checked();
    //    base::operator()(&checkedExpr);
    //    auto checkedVal = checkedExpr.value();
    //    auto opcode = matcher.has_implicit_op() ?
    //      val_ops::Equal :
    //      detail::conv_binary(matcher.pos().m_kind);

    //    currentMatch = m_visitor.visit_binary(&matcher, condVal, checkedVal, opcode);
    //  }

    //  matcher.eval_result(currentMatch);
    //  if (to_bool(currentMatch))
    //  {
    //    trueBranch = &pattern;
    //    break;
    //  }
    //}

    //if (auto winner = (trueBranch ? trueBranch : defaultBranch))
    //{
    //  if (auto&& body = winner->body(); !body.children().empty())
    //    base::operator()(&winner->body());
    //  else
    //    m_visitor.get_empty();

    //  expr.eval_result(m_visitor.last_result(&expr));
    //}
    //else
    //{
    //  expr.eval_result(m_visitor.get_empty());
    //}

    return false;
  }

  bool evaluator::preview(ast::cond_short& expr) noexcept
  {
    utils::unused(expr);
    //if (return_path())
    //  return false;

    //auto&& cond = expr.cond();
    //base::operator()(&cond);
    //ast::expr* winner{};
    //if (auto condVal = cond.value(); to_bool(condVal))
    //{
    //  if (!expr.has_true())
    //  {
    //    expr.eval_result(m_visitor.visit_assign(&expr, condVal));
    //    return false;
    //  }
    //  winner = &expr.on_true();
    //}
    //else
    //{
    //  if (!expr.has_false())
    //  {
    //    expr.eval_result(m_visitor.get_empty());
    //    return false;
    //  }
    //  winner = &expr.on_false();
    //}

    //UTILS_ASSERT(winner);
    //base::operator()(winner);
    //expr.eval_result(m_visitor.visit_assign(&expr, winner->value()));

    return false;
  }

  bool evaluator::preview(ast::scope& scope) noexcept
  {
    utils::unused(scope); return false;
    //const auto returning = return_path();
    //auto callable = try_get_callable(scope);
    //if (!callable)
    //  return !returning;

    //if(!returning)
    //  m_callStack.prologue(*callable, m_visitor);

    //return !returning;
  }

  void evaluator::visit(ast::scope& scope) noexcept
  {
    utils::unused(scope);
    //auto callable = try_get_callable(scope);
    //if (!callable)
    //  return;

    //auto resVal = m_visitor.last_result(&callable->declarator());
    //m_callStack.epilogue(*callable, m_visitor);
    //m_visitor.visit_assign(nullptr, resVal);
  }

  bool evaluator::exit_child() noexcept
  {
    m_visitor.fetch_next();
    return !return_path();
  }

  // Private members

  void evaluator::traverse(ast::node* root) noexcept
  {
    base::operator()(root);
  }

  void evaluator::on_error(const token& pos, string_t msg) noexcept
  {
    if (m_errHandler)
      m_errHandler(pos, msg);
  }

  void evaluator::eval_token(const token& tok) noexcept
  {
    switch (tok.m_kind)
    {
    case token::KwTrue:  m_visitor.visit_bool_literal(true);           break;
    case token::KwFalse: m_visitor.visit_bool_literal(false);          break;
    case token::IntDec:  m_visitor.visit_int_literal(tok.m_value, 10); break;
    case token::IntBin:  m_visitor.visit_int_literal(tok.m_value, 2);  break;
    case token::IntOct:  m_visitor.visit_int_literal(tok.m_value, 8);  break;
    case token::IntHex:  m_visitor.visit_int_literal(tok.m_value, 16); break;
    case token::Float:   m_visitor.visit_float_literal(tok.m_value);   break;

    default: break;
    }
  }

  void evaluator::eval_assign(semantics::symbol& sym, eval::value rhs) noexcept
  {
    utils::unused(sym, rhs);
    // sym.eval_result(m_visitor.visit_assign(&sym, rhs));
  }

  void evaluator::make_function(semantics::function& sym) noexcept
  {
    utils::unused(sym);
    // sym.eval_result(m_visitor.make_function(&sym, eval::function_type{ sym }));
  }

  void evaluator::make_arr_call(eval::array_type arr, ast::call_expr& expr) noexcept
  {
    utils::unused(arr, expr);
    //auto&& callRes = m_visitor.new_array(&expr, arr->size());
    //auto&& args = expr.args();
    //const auto argCount = args.size();

    //for (auto elem : *arr)
    //{
    //  auto argFunc = elem.try_get<eval::function_type>();
    //  if (!argFunc || (*argFunc)->param_count() != argCount)
    //    continue;

    //  make_call(argFunc, expr);
    //  auto&& elemVal = callRes.emplace_back();
    //  elemVal = m_visitor.last_result(&elemVal);
    //}

    //m_visitor.make_array(&expr, callRes);
  }

  void evaluator::make_call(eval::function_type* func, ast::call_expr& expr) noexcept
  {
    utils::unused(func, expr);
    //auto&& at = expr.pos();
    //if (!func)
    //{
    //  on_error(at, "Expected a callable object"sv);
    //  m_visitor.get_empty();
    //  return;
    //}

    //auto callable = *func;
    //auto&& args = expr.args();
    //if (const auto paramCnt = callable->param_count(); paramCnt != args.size())
    //{
    //  on_error(at, std::format("Expected {} arguments"sv, paramCnt));
    //  return;
    //}

    //if (!m_callStack)
    //{
    //  on_error(at, "Stack overflow"sv);
    //  m_callStack.clear();
    //  m_visitor.get_empty();
    //  return;
    //}

    //m_callStack.push(*callable, args, m_visitor);
    //auto funcBody = callable->declarator().definition();
    //value_guard _{ m_return };
    //(*this)(funcBody);
  }

  bool evaluator::return_path() const noexcept
  {
    return m_return;
  }

  bool evaluator::to_bool(eval::value val) const noexcept
  {
    auto resVal = eval::cast_value<eval::bool_type>(val);
    UTILS_ASSERT(resVal.has_value());
    return *resVal;
  }

  semantics::function* evaluator::try_get_callable(ast::scope& scope) const noexcept
  {
    using semantics::sym_kind;
    if(auto func = utils::try_cast<ast::func_decl>(scope.parent()))
      return utils::try_cast<sym_kind::Function>(&func->symbol());

    return {};
  }
}