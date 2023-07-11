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

        eval::value operator()(const ast::typed_expr& expr) noexcept
        {
          if (!check_args(expr))
          {
            return {};
          }

          return instantiate(expr, std::make_index_sequence<max>{});
        }

      private:
        template <typename T, T... Seq>
        eval::value instantiate(const ast::typed_expr& expr, std::integer_sequence<T, Seq...>) noexcept
        {
          auto&& exprArgs = expr.args();
          return m_visitor.instantiate<value_type>(&expr, extract(exprArgs, Seq)...);
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

        eval::value extract(const arg_list_t& args, size_type idx) noexcept
        {
          const auto count = args.size();
          return idx < count ? args[idx]->value() : eval::value{};
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
    m_return = false;
    base::operator()(root);
  }

  // Expressions

  void evaluator::visit(ast::assign_expr& assign) noexcept
  {
    if (return_path())
      return;

    auto&& left = assign.left();
    if (auto assignee = utils::try_cast<ast::id_expr>(&left))
    {
      auto&& lhs = assignee->symbol();
      eval_assign(lhs, assign.right().value());
      assignee->eval_result(lhs.value());
    }

    assign.eval_result(left.value());
  }

  void evaluator::visit(ast::binary_expr& binary) noexcept
  {
    if (return_path())
      return;

    const auto opcode = binary.op().m_kind;

    // We've already calculated logical && and || at this point
    if (detail::is_logical(opcode))
      return;

    auto left = binary.left().value();
    auto right = binary.right().value();
    const auto opCode = detail::conv_binary(opcode);
    binary.eval_result(m_visitor.visit_binary(&binary, left, right, opCode));
  }

  void evaluator::visit(ast::unary_expr& unary) noexcept
  {
    if (return_path())
      return;

    const auto opCode = detail::conv_unary(unary.op().m_kind);
    auto val = unary.operand().value();
    unary.eval_result(m_visitor.visit_unary(val, opCode));
  }

  void evaluator::visit(ast::typed_expr& expr) noexcept
  {
    if (return_path())
      return;

    using enum tok_kind;
    eval::value val;
    using detail::instance;

    switch (expr.type_name().m_kind)
    {
    case KwComplex:  val = instance<eval::complex_type>{ m_visitor, m_errHandler }(expr);  break;
    case KwFraction: val = instance<eval::fraction_type>{ m_visitor, m_errHandler }(expr); break;
    
    default: UTILS_ASSERT(false); break;
    }

    expr.eval_result(val);
  }

  void evaluator::visit(ast::call_expr& expr) noexcept
  {
    if (return_path())
      return;

    auto func = expr.callable().value();
    if (auto arr = func.try_get<eval::array_type>())
    {
      make_arr_call(*arr, expr);
    }
    else
    {
      auto funcType = func.try_get<eval::function_type>();
      make_call(funcType, expr.pos(), expr.args());
    }

    auto res = m_visitor.last_result(&expr);
    expr.eval_result(res);
  }

  void evaluator::visit(ast::ret_expr& ret) noexcept
  {
    if (return_path())
      return;

    auto returnedVal = m_visitor.visit_assign(&ret, ret.returned_value().value());
    ret.eval_result(returnedVal);
    m_return = true;
  }

  void evaluator::visit(ast::array_expr& arr) noexcept
  {
    if (return_path())
      return;

    auto&& elements = arr.elements();
    const auto arrSz = elements.size();
    auto&& newArr = m_visitor.new_array(&arr, arrSz);
    for (auto elem : elements)
    {
      newArr.emplace_back(elem->value());
    }
    arr.eval_result(m_visitor.make_array(&arr, newArr));
  }

  void evaluator::visit(ast::paren_expr& paren) noexcept
  {
    if (return_path())
      return;

    paren.eval_result(paren.internal_expr().value());
  }

  void evaluator::visit(ast::abs_expr& abs) noexcept
  {
    if (return_path())
      return;

    auto&& intExpr = abs.expression();
    const auto opcode = eval::val_ops::AbsoluteValue;
    abs.eval_result(m_visitor.visit_unary(intExpr.value(), opcode));
  }

  void evaluator::visit(ast::lit_expr& lit) noexcept
  {
    if (return_path())
      return;

    if (lit.value())
      return;

    auto value = eval_token(lit.pos());
    lit.eval_result(value);
  }

  void evaluator::visit(ast::id_expr& id) noexcept
  {
    if (return_path())
      return;

    auto&& sym = id.symbol();
    auto val = sym.value();
    m_visitor.visit_assign(nullptr, val);
    id.eval_result(val);
  }

  void evaluator::visit(ast::result_expr& res) noexcept
  {
    if (return_path())
      return;

    res.eval_result(m_visitor.last_result(&res));
  }

  // Decls

  void evaluator::visit(ast::decl_expr& expr) noexcept
  {
    if (return_path())
      return;

    auto&& sym = expr.declarator().symbol();
    auto val = sym.value();
    m_visitor.visit_assign(nullptr, val);
    expr.eval_result(val);
  }

  void evaluator::visit(ast::var_decl& decl) noexcept
  {
    if (return_path())
      return;

    eval_assign(decl.symbol(), decl.initialiser().value());
  }

  void evaluator::visit(ast::func_decl& decl) noexcept
  {
    if (return_path())
      return;

    auto sym = utils::try_cast<semantics::function>(&decl.symbol());

    if (!sym)
    {
      on_error(decl.pos(), "Invalid symbol type"sv);
      return;
    }

    if (sym->value())
      return;

    make_function(*sym);
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
    base::operator()(&lhs);

    auto lval = to_bool(lhs.value());

    bool res{};
    if ((!lval && opcode == tok_kind::LogAnd) ||
        ( lval && opcode == tok_kind::LogOr))
    {
      res = lval;
    }
    else
    {
      auto&& rhs = expr.right();
      base::operator()(&rhs);
      auto rval  = to_bool(rhs.value());
      res = (opcode == tok_kind::LogAnd) ? (lval && rval) : (lval || rval);
    }

    expr.eval_result(m_visitor.visit_bool_literal(res));
    return false;
  }

  bool evaluator::preview(ast::cond_expr& expr) noexcept
  {
    if (return_path())
      return false;

    auto&& cond = expr.cond();
    base::operator()(&cond);
    auto condVal = cond.value();

    ast::pattern* trueBranch{};
    ast::pattern* defaultBranch{};
    using eval::val_ops;
    for (auto child : expr.patterns().children())
    {
      auto&& pattern = utils::cast<ast::pattern>(*child);
      auto&& matcher = utils::cast<ast::matcher>(pattern.matcher());
      eval::value currentMatch{};
      if (matcher.is_default())
      {
        defaultBranch = &pattern;
        continue;
      }
      else if (matcher.is_unary())
      {
        const auto opcode = detail::conv_unary(matcher.pos().m_kind);
        currentMatch = m_visitor.visit_unary(condVal, opcode);
      }
      else
      {
        auto&& checkedExpr = matcher.checked();
        base::operator()(&checkedExpr);
        auto checkedVal = checkedExpr.value();
        auto opcode = matcher.has_implicit_op() ?
          val_ops::Equal :
          detail::conv_binary(matcher.pos().m_kind);

        currentMatch = m_visitor.visit_binary(&matcher, condVal, checkedVal, opcode);
      }

      matcher.eval_result(currentMatch);
      if (to_bool(currentMatch))
      {
        trueBranch = &pattern;
        break;
      }
    }

    if (auto winner = (trueBranch ? trueBranch : defaultBranch))
    {
      if (auto&& body = winner->body(); !body.children().empty())
        base::operator()(&winner->body());
      else
        m_visitor.get_empty();

      expr.eval_result(m_visitor.last_result(&expr));
    }
    else
    {
      expr.eval_result(m_visitor.get_empty());
    }

    return false;
  }

  bool evaluator::preview(ast::cond_short& expr) noexcept
  {
    if (return_path())
      return false;

    auto&& cond = expr.cond();
    base::operator()(&cond);
    ast::expr* winner{};
    if (auto condVal = cond.value(); to_bool(condVal))
    {
      if (!expr.has_true())
      {
        expr.eval_result(m_visitor.visit_assign(&expr, condVal));
        return false;
      }
      winner = &expr.on_true();
    }
    else
    {
      if (!expr.has_false())
      {
        expr.eval_result(m_visitor.get_empty());
        return false;
      }
      winner = &expr.on_false();
    }

    UTILS_ASSERT(winner);
    base::operator()(winner);
    expr.eval_result(m_visitor.visit_assign(&expr, winner->value()));

    return false;
  }

  bool evaluator::preview(ast::scope& scope) noexcept
  {
    const auto returning = return_path();
    auto callable = try_get_callable(scope);
    if (!callable)
      return !returning;

    if(!returning)
      m_callStack.prologue(*callable, m_visitor);

    return !returning;
  }

  void evaluator::visit(ast::scope& scope) noexcept
  {
    auto callable = try_get_callable(scope);
    if (!callable)
      return;

    auto resVal = m_visitor.last_result(&callable->declarator());
    m_callStack.epilogue(*callable, m_visitor);
    m_visitor.visit_assign(nullptr, resVal);
  }

  // Private members

  void evaluator::on_error(const token& pos, string_t msg) noexcept
  {
    if (m_errHandler)
      m_errHandler(pos, msg);
  }

  eval::value evaluator::eval_token(const token& tok) noexcept
  {
    switch (tok.m_kind)
    {
    case token::KwTrue:  return m_visitor.visit_bool_literal(true);
    case token::KwFalse: return m_visitor.visit_bool_literal(false);
    case token::IntDec:  return m_visitor.visit_int_literal(tok.m_value, 10);
    case token::IntBin:  return m_visitor.visit_int_literal(tok.m_value, 2);
    case token::IntOct:  return m_visitor.visit_int_literal(tok.m_value, 8);
    case token::IntHex:  return m_visitor.visit_int_literal(tok.m_value, 16);
    case token::Float:   return m_visitor.visit_float_literal(tok.m_value);

    default: return {};
    }
  }

  void evaluator::eval_assign(semantics::symbol& sym, eval::value rhs) noexcept
  {
    sym.eval_result(m_visitor.visit_assign(&sym, rhs));
  }

  void evaluator::make_function(semantics::function& sym) noexcept
  {
    sym.eval_result(m_visitor.make_function(&sym, eval::function_type{ sym }));
  }

  void evaluator::make_arr_call(eval::array_type arr, ast::call_expr& expr) noexcept
  {
    auto&& callRes = m_visitor.new_array(&expr, arr->size());
    auto&& args = expr.args();
    const auto argCount = args.size();
    const auto callPos = expr.pos();
    const auto arrId = *eval::detail::ent_id{ &callRes };
    for (auto idx = size_type{}; auto elem : *arr)
    {
      auto argFunc = elem.try_get<eval::function_type>();
      if (!argFunc || (*argFunc)->param_count() != argCount)
        continue;

      make_call(argFunc, callPos, args);

      const auto elemId = arrId + idx;
      auto elemVal = m_visitor.last_result(elemId);
      callRes.emplace_back(elemVal);
      ++idx;
    }

    m_visitor.make_array(&expr, callRes);
  }

  void evaluator::make_call(eval::function_type* func, const token& at, ast::call_expr::arg_list& args) noexcept
  {
    if (!func)
    {
      on_error(at, "Expected a callable object"sv);
      m_visitor.get_empty();
      return;
    }

    auto callable = *func;
    if (const auto paramCnt = callable->param_count(); paramCnt != args.size())
    {
      on_error(at, std::format("Expected {} arguments"sv, paramCnt));
      return;
    }

    if (!m_callStack)
    {
      on_error(at, "Stack overflow"sv);
      m_callStack.clear();
      m_visitor.get_empty();
      return;
    }

    m_callStack.push(*callable, args, m_visitor);
    auto funcBody = callable->declarator().definition();
    value_guard _{ m_return };
    (*this)(funcBody);
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