#pragma once
#include "ast/ast_visitor.hpp"

namespace tnac
{
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
    using enum tnac::eval::type_id;

    auto print = [](auto&& val, std::uintptr_t addr = 0) noexcept
    {
      if (!val)
      {
        std::cout << "<undef>";
        return;
      }

      std::cout << "0x" << std::hex << addr;
      std::cout << std::dec << ": " << *val;
    };

    switch (v.id())
    {
    case Int:
      print(v.try_get<tnac::int_type>(), v.raw_value());
      break;

    case Float:
      print(v.try_get<tnac::float_type>(), v.raw_value());
      break;

    default:
      print(std::optional<int>{ std::nullopt });
      break;
    }
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

      print_token(op);
    }

    void visit(const ast::unary_expr* expr) noexcept
    {
      indent(expr);
      auto&& op = expr->op();
      std::cout << "Unary expression: ";
      print_token(op);
    }

    void visit(const ast::paren_expr* expr) noexcept
    {
      indent(expr);
      std::cout << "Paren expr\n";
    }

    void visit(const ast::lit_expr* expr) noexcept
    {
      indent(expr);
      auto&& pos = expr->pos();
      auto val = expr->value();
      std::cout << "Literal expression: (";
      print_value(val);
      std::cout << "), ";
      print_token(pos);
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