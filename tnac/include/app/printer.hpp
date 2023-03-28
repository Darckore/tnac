#pragma once
#include "ast/ast_visitor.hpp"
#include "evaluator/value.hpp"

namespace printer
{
  using namespace tnac;

  inline std::ostream& operator<<(std::ostream& out, eval::invalid_val_t) noexcept
  {
    out << "<undef>";
    return out;
  }

  void print_token(const tnac::token& tok) noexcept
  {
    static constexpr std::array kinds{
      "Invalid token"sv,
      "Eol"sv,
      "ExprSep"sv,
      "IntBin"sv,
      "IntOct"sv,
      "IntDec"sv,
      "IntHex"sv,
      "Float"sv,
      "Plus"sv,
      "Minus"sv,
      "Asterisk"sv,
      "Slash"sv,
      "ParenOpen"sv,
      "ParenClose"sv,
      "Identifier"sv,
      "Command"sv,
      "keyword"sv,
    };

    using idx_t = decltype(kinds)::size_type;

    std::cout << kinds[static_cast<idx_t>(tok.m_kind)] << ": '" << tok.m_value << "'\n";
  }

  void print_value(tnac::eval::value v) noexcept
  {
    eval::on_value(v, [addr = v.raw_value()](auto val)
      {
        std::cout << "(0x" << std::hex << addr << std::dec << ": " << val << "), ";
      });
  }

  void print_newline() noexcept
  {
    std::cout << '\n';
  }

  template <typename Derived>
  using printer_base = ast::const_top_down_visitor<Derived>;

  class ast_printer : public printer_base<ast_printer>
  {
  public:
    CLASS_SPECIALS_ALL(ast_printer);

  public:
    void visit(const ast::scope* scope) noexcept
    {
      indent(scope);
      std::cout << "<scope>\n";
    }

    void visit(const ast::binary_expr* expr) noexcept
    {
      indent(expr);
      auto&& op = expr->op();

      using tnac::token;
      if (op.is_any(token::Plus, token::Minus))
        std::cout << "Additive expression: ";
      else if (op.is_any(token::Asterisk, token::Slash))
        std::cout << "Multiplicative expression: ";
      else
        std::cout << "Binary expression: ";

      print_value(expr->value());
      print_token(op);
    }

    void visit(const ast::unary_expr* expr) noexcept
    {
      indent(expr);
      std::cout << "Unary expression: ";
      print_value(expr->value());
      print_token(expr->op());
    }

    void visit(const ast::paren_expr* expr) noexcept
    {
      indent(expr);
      std::cout << "Paren expr";
      print_value(expr->value());
      std::cout << '\n';
    }

    void visit(const ast::lit_expr* expr) noexcept
    {
      indent(expr);
      std::cout << "Literal expression: ";
      print_value(expr->value());
      print_token(expr->pos());
    }

    void visit(const ast::id_expr* expr) noexcept
    {
      indent(expr);
      std::cout << "Id expression: " << expr->name() << '\n';
    }

    void visit(const ast::error_expr* expr) noexcept
    {
      indent(expr);
      auto&& at = expr->at();
      auto msg = expr->message();
      std::cout << "Error '" << msg << "' at ";
      print_token(at);
    }

  private:
    void indent(const ast::node* cur) const
    {
      auto depth = 0;
      while (auto parent = cur->parent())
      {
        cur = parent;
        depth += 2;
      }

      for (auto i = 1; i < depth; ++i)
        std::cout << ' ';
      std::cout << '|';
    }
  };
}