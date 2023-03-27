#include <iomanip>

#include "app/printer.hpp"
#include "parser/parser.hpp"
#include "evaluator/value.hpp"
#include "evaluator/evaluator.hpp"

#define PRINT_TOKENS 0

bool parse_line(tnac::buf_t input) noexcept
{
  static std::forward_list<tnac::buf_t> lineBuf;
  static tnac::lex lex;
  static tnac::ast::builder builder;
  static tnac::parser parser{ builder };
  static tnac::eval::registry registry;
  static tnac::evaluator evaluator{ registry };

  std::cout << "Input: '" << input << "'\n\n";
  lineBuf.emplace_front(std::move(input));

#if PRINT_TOKENS
  std::cout << "Tokens:\n";
  lex(lineBuf.front());
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

  if (auto ast = parser(lineBuf.front()))
  {
    evaluator(ast);

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
  using vt = tnac::eval::type_id;

  std::intmax_t x = 42;
  value vv{ &x, vt::Int };
  const auto id = vv.id();
  const auto val = vv.raw_value();
  auto retVal = vv.try_get<std::intmax_t>();
  utils::unused(id, val, retVal);

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