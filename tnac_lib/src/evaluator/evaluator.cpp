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
        switch (tk)
        {
        case Plus:
          return eval::val_ops::UnaryPlus;

        case Minus:
          return eval::val_ops::UnaryNegation;
          
        default:
          return eval::val_ops::InvalidOp;
        }
      }
      constexpr auto conv_binary(tok_kind tk) noexcept
      {
        using enum tok_kind;
        switch (tk)
        {
        case Plus:
          return eval::val_ops::Addition;

        case Minus:
          return eval::val_ops::Subtraction;

        case Asterisk:
          return eval::val_ops::Multiplication;

        case Slash:
          return eval::val_ops::Division;

        case Percent:
          return eval::val_ops::Modulo;

        default:
          return eval::val_ops::InvalidOp;
        }
      }
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
    auto&& typeName = expr.type_name();
    auto&& args     = expr.params();
    eval::value val;

    switch (typeName.m_kind)
    {
    case KwComplex:
      if (check_args(typeName, args, 0, 2))
      {
        val = m_visitor.instantiate<complex_type>(&expr, extract(args, 0), extract(args, 1));
      }
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

  bool evaluator::check_args(const token& tok, const param_list_t& args, size_type min, size_type max) noexcept
  {
    const auto size = args.size();
    if (utils::in_range(size, min, max))
      return true;

    if (size < min)
      on_error(tok, "Too few arguments"sv);

    if (size > max)
      on_error(tok, "Too many arguments"sv);

    return false;
  }

  eval::value evaluator::extract(const param_list_t& args, size_type idx) noexcept
  {
    const auto count = args.size();
    return idx < count ? args[idx]->value() : eval::value{};
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