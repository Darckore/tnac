#include "parser/parser.hpp"
#include "ast/ast_visitor.hpp"

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

class printing_visitor : public tnac::ast::visitor<printing_visitor, const tnac::ast::node>
{
public:
  using base = tnac::ast::visitor<printing_visitor, const tnac::ast::node>;
  using value_type = const tnac::ast::node;
  using ptr_type = value_type*;

  CLASS_SPECIALS_NONE(printing_visitor);

  printing_visitor(ptr_type root) :
    base{ root }
  {}

public:
  void visit(const tnac::ast::scope* scope) noexcept
  {
    indent(scope);
    std::cout << "<scope>\n";
  }

  void visit(const tnac::ast::binary_expr* expr) noexcept
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

  void visit(const tnac::ast::unary_expr* expr) noexcept
  {
    indent(expr);
    auto&& op = expr->op();
    std::cout << "Unary expression: ";
    print_token(op);
  }

  void visit(const tnac::ast::paren_expr* expr) noexcept
  {
    indent(expr);
    std::cout << "Paren expr\n";
  }

  void visit(const tnac::ast::lit_expr* expr) noexcept
  {
    indent(expr);
    auto&& val = expr->value();
    std::cout << "Literal expression: ";
    print_token(val);
  }

  void visit(const tnac::ast::id_expr* expr) noexcept
  {
    indent(expr);
    std::cout << "Id expression: " << expr->name() << '\n';
  }

  void visit(const tnac::ast::error_expr* expr) noexcept
  {
    indent(expr);
    auto&& at = expr->at();
    auto msg = expr->message();
    std::cout << "Error '" << msg << "' at ";
    print_token(at);
  }

private:
  void indent(ptr_type cur) const
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


#define PRINT_TOKENS 0

bool parse_line(tnac::buf_t input) noexcept
{
  static std::forward_list<tnac::buf_t> lineBuf;
  static tnac::lex lex;
  static tnac::parser parser;

  std::cout << "Input: '" << input << "'\n\n";
  lineBuf.emplace_front(std::move(input));

#if PRINT_TOKENS
  std::cout << "Tokens:\n";
  lex.feed(lineBuf.front());
  for (;;)
  {
    auto tok = lex.next();
    if (tok.is_eol())
    {
      std::cout << "End of input\n\n";
      break;
    }

    print_token(tok);
  }
#endif

  if (auto ast = parser.parse(lineBuf.front()))
  {
    printing_visitor pv{ ast };
  }
  else
  {
    std::cout << "Failed to generate AST\n\n";
  }

  return true;
}

int main()
{
  tnac::buf_t input;
  while (true)
  {
    std::cout << ">> ";
    std::getline(std::cin, input);
    if (utils::ltrim(input).empty())
    {
      std::cout << "Enter an expression\n";
      continue;
    }

    if (!parse_line(std::move(input)))
      break;

    input = {};
  }

  return 0;
}