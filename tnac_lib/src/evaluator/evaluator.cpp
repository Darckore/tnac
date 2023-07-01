#include "evaluator/evaluator.hpp"
#include "evaluator/value.hpp"
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

    auto left = binary.left().value();
    auto right = binary.right().value();
    const auto opCode = detail::conv_binary(binary.op().m_kind);
    binary.eval_result(m_visitor.visit_binary(&binary, left, right, opCode));
  }

  void evaluator::visit(ast::unary_expr& unary) noexcept
  {
    if (return_path())
      return;

    const auto opCode = detail::conv_unary(unary.op().m_kind);
    auto val = unary.operand().value();
    unary.eval_result(m_visitor.visit_unary(&unary, val, opCode));
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
    case KwComplex:
      val = instance<eval::complex_type>{ m_visitor, m_errHandler }(expr);
      break;

    case KwFraction:
      val = instance<eval::fraction_type>{ m_visitor, m_errHandler }(expr);
      break;

    default:
      UTILS_ASSERT(false);
      break;
    }

    expr.eval_result(val);
  }

  void evaluator::visit(ast::call_expr& expr) noexcept
  {
    if (return_path())
      return;

    auto func = expr.callable().value();
    auto funcType = func.try_get<eval::function_type>();
    if (!funcType)
    {
      on_error(expr.pos(), "Expected a callable object"sv);
      return;
    }

    auto callable = *funcType;
    if (const auto paramCnt = callable->param_count(); paramCnt != expr.args().size())
    {
      on_error(expr.pos(), std::format("Expected {} arguments"sv, paramCnt));
      return;
    }

    if (!init_call(*callable, expr))
    {
      on_error(expr.pos(), "Stack overflow"sv);
      m_callStack.clear();
      expr.eval_result(m_visitor.get_empty());
      return;
    }

    auto funcBody = callable->declarator().definition();
    value_guard _{ m_return };
    (*this)(funcBody);
    auto val = m_visitor.last_result(&expr);
    expr.eval_result(val);
    m_callStack.pop();
  }

  void evaluator::visit(ast::ret_expr& ret) noexcept
  {
    if (return_path())
      return;

    ret.eval_result(ret.returned_value().value());
    m_return = true;
  }

  void evaluator::visit(ast::paren_expr& paren) noexcept
  {
    if (return_path())
      return;

    paren.eval_result(paren.internal_expr().value());
  }

  void evaluator::visit(ast::lit_expr& lit) noexcept
  {
    if (return_path())
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

  bool evaluator::preview(ast::call_expr& call) noexcept
  {
    if (return_path())
      return false;

    auto&& ev = *this;
    for (auto arg : call.args())
    {
      ev(arg);
    }
    ev(&call.callable());
    return false;
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
    case token::KwTrue:
      return m_visitor.visit_bool_literal(true);
    case token::KwFalse:
      return m_visitor.visit_bool_literal(false);

    case token::IntDec:
      return m_visitor.visit_int_literal(tok.m_value, 10);
    case token::IntBin:
      return m_visitor.visit_int_literal(tok.m_value, 2);
    case token::IntOct:
      return m_visitor.visit_int_literal(tok.m_value, 8);
    case token::IntHex:
      return m_visitor.visit_int_literal(tok.m_value, 16);

    case token::Float:
      return m_visitor.visit_float_literal(tok.m_value);

    default:
      return {};
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

  bool evaluator::init_call(semantics::function& sym, ast::call_expr& expr) noexcept
  {
    if (!m_callStack)
    {
      return false;
    }

    eval::call_stack::value_list argValues;
    argValues.reserve(expr.args().size());

    for (auto [param, arg] : utils::make_iterators(sym.params(), expr.args()))
    {
      UTILS_ASSERT(static_cast<bool>(param));
      auto&& paramSym = param->symbol();

      auto val = arg->value();
      argValues.emplace_back(val);
      eval_assign(paramSym, val);
    }

    m_visitor.get_empty();
    return m_callStack.push(sym.name(), std::move(argValues));
  }

  bool evaluator::return_path() const noexcept
  {
    return m_return;
  }
}