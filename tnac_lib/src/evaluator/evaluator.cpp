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
    id.eval_result(id.symbol().value());
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