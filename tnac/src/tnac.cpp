#include "parser/parser.hpp"

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
    "Slash"sv
  };

  using idx_t = decltype(kinds)::size_type;

  std::cout << kinds[static_cast<idx_t>(tok.m_kind)] << ": '" << tok.m_value  << "'\n";
}

void print_ast(tnac::ast::node& node, int depth = 0) noexcept
{
  namespace ast = tnac::ast;
  for (auto i = 1; i < depth; ++i)
    std::cout << ' ';
  std::cout << '|';

  using enum tnac::ast::node_kind;
  switch (node.what())
  {
  case Scope:
    break;

  case Literal:
  {
    auto&& lit = static_cast<ast::lit_expr&>(node);
    auto&& val = lit.value();
    std::cout << "Literal expression: ";
    print_token(val);
  }
    break;

  case Unary:
  {
    auto&& unary = static_cast<ast::unary_expr&>(node);
    auto&& op = unary.op();
    auto&& operand = unary.operand();
    std::cout << "Unary expression: ";
    print_token(op);
    print_ast(operand, depth + 2);
  }
    break;

  case Binary:
  {
    auto&& binary = static_cast<ast::binary_expr&>(node);
    auto&& op = binary.op();
    auto&& left = binary.left();
    auto&& right = binary.right();
    
    using tnac::token;
    if (op.is_any(token::Plus, token::Minus))
      std::cout << "Additive expression: ";
    else if (op.is_any(token::Asterisk, token::Slash))
      std::cout << "Multiplicative expression: ";
    else
      std::cout << "Binary expression: ";
    
    print_token(op);
    print_ast(left, depth + 2);
    print_ast(right, depth + 2);
  }
    break;

  default:
    std::cout << "Invalid or unknown node";
    break;
  }
}

bool parse_line(tnac::buf_t input) noexcept
{
  static std::forward_list<tnac::buf_t> lineBuf;
  static tnac::lex lex;
  static tnac::parser parser;

  std::cout << "Input: '" << input << "'\n\nTokens:\n";

  lineBuf.emplace_front(std::move(input));
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

  auto ast = parser.parse(lineBuf.front());
  if (!ast)
  {
    std::cout << "Failed to generate AST\n\n";
    return true;
  }

  print_ast(*ast);
  return true;
}

int main()
{
  tnac::buf_t input;
  while (true)
  {
    std::cout << ">> ";
    std::getline(std::cin, input);
    if (input.empty())
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