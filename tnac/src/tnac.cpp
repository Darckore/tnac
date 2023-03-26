#include "app/printer.hpp"
#include "parser/parser.hpp"
#include "evaluator/value.hpp"

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
    using tnac::ast_printer;
    ast_printer{}(ast);
  }
  else
  {
    std::cout << "Failed to generate AST\n\n";
  }

  return true;
}

int main()
{
  using tnac::eval::value;
  using vt = value::type_id;

  std::intmax_t x = 42;
  value vv{ &x, vt::Int };
  const auto id = vv.id();
  const auto val = vv.raw_value();
  utils::unused(id, val);

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