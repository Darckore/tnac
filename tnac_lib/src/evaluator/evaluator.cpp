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
        case Plus:
          return UnaryPlus;

        case Minus:
          return UnaryNegation;

        case Tilde:
          return UnaryBitwiseNot;
          
        default:
          return InvalidOp;
        }
      }
      constexpr auto conv_binary(tok_kind tk) noexcept
      {
        using enum tok_kind;
        using enum eval::val_ops;
        switch (tk)
        {
        case Plus:
          return Addition;

        case Minus:
          return Subtraction;

        case Asterisk:
          return Multiplication;

        case Slash:
          return Division;

        case Percent:
          return Modulo;

        case Amp:
          return BitwiseAnd;

        case Hat:
          return BitwiseXor;

        case Pipe:
          return BitwiseOr;

        default:
          return InvalidOp;
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
        using param_list_t = evaluator::param_list_t;
        using size_type = param_list_t::size_type;
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
          auto&& exprArgs = expr.params();
          return m_visitor.instantiate<value_type>(&expr, extract(exprArgs, Seq)...);
        }

        void on_error(const token& pos, string_t msg) noexcept
        {
          if (m_errHandler)
            m_errHandler(pos, msg);
        }

        bool check_args(const ast::typed_expr& expr) noexcept
        {
          const auto size = expr.params().size();
          if (utils::in_range(size, min, max))
            return true;

          auto msg = std::format("Expected at least {} and at most {} arguments"sv, min, max);
          on_error(expr.type_name(), msg);

          return false;
        }

        eval::value extract(const param_list_t& args, size_type idx) noexcept
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

  evaluator::evaluator(eval::registry& registry) noexcept :
    m_visitor{ registry }
  {}


  // Public members

  // Expressions

  void evaluator::visit(ast::assign_expr& assign) noexcept
  {
    using enum ast::node::kind;
    auto&& assignee = assign.left();
    switch (assignee.what())
    {
    case Identifier:
    {
      auto&& lhs = static_cast<ast::id_expr&>(assignee).symbol();
      eval_assign(lhs, assign.right().value());
      assignee.eval_result(lhs.value());
    }
      break;

    default:
      break;
    }

    assign.eval_result(assignee.value());
  }

  void evaluator::visit(ast::binary_expr& binary) noexcept
  {
    auto left = binary.left().value();
    auto right = binary.right().value();
    const auto opCode = detail::conv_binary(binary.op().m_kind);
    binary.eval_result(m_visitor.visit_binary(&binary, left, right, opCode));
  }

  void evaluator::visit(ast::unary_expr& unary) noexcept
  {
    const auto opCode = detail::conv_unary(unary.op().m_kind);
    auto val = unary.operand().value();
    unary.eval_result(m_visitor.visit_unary(&unary, val, opCode));
  }

  void evaluator::visit(ast::typed_expr& expr) noexcept
  {
    using enum tok_kind;
    eval::value val;
    using detail::instance;

    switch (expr.type_name().m_kind)
    {
    case KwComplex:
      val = instance<complex_type>{ m_visitor, m_errHandler }(expr);
      break;

    case KwFraction:
      val = instance<fraction_type>{ m_visitor, m_errHandler }(expr);
      break;

    default:
      UTILS_ASSERT(false);
      break;
    }

    expr.eval_result(val);
  }

  void evaluator::visit(ast::paren_expr& paren) noexcept
  {
    paren.eval_result(paren.internal_expr().value());
  }

  void evaluator::visit(ast::lit_expr& lit) noexcept
  {
    auto value = eval_token(lit.pos());
    lit.eval_result(value);
  }

  void evaluator::visit(ast::id_expr& id) noexcept
  {
    auto&& sym = id.symbol();
    auto val = sym.value();
    m_visitor.visit_assign(nullptr, val);
    id.eval_result(val);
  }

  void evaluator::visit(ast::result_expr& res) noexcept
  {
    res.eval_result(m_visitor.last_result(&res));
  }

  // Decls

  void evaluator::visit(ast::decl_expr& expr) noexcept
  {
    auto&& sym = expr.declarator().symbol();
    expr.eval_result(sym.value());
  }

  void evaluator::visit(ast::var_decl& decl) noexcept
  {
    eval_assign(decl.symbol(), decl.definition().value());
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
}