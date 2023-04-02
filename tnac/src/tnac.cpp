#include <iomanip>

#include "app/printer.hpp"
#include "tnac/driver.hpp"

#define PRINT_TOKENS 0

bool parse_line(tnac::buf_t input) noexcept
{
  static std::forward_list<tnac::buf_t> lineBuf;
  static tnac::lex lex;
  static tnac::ast::builder builder;
  static tnac::sema sema;
  static tnac::parser parser{ builder, sema };
  static tnac::eval::registry registry;
  static tnac::evaluator evaluator{ registry };
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
    using printer::ast_printer;
    using printer::print_newline;

    print_newline();

    evaluator(ast);
    ast_printer{}(ast);
    
    print_newline();
  }
  else
  {
    std::cout << "Failed to generate AST\n\n";
  }

  return true;
}

int main()
{
  //tnac::driver driver{ std::cout, std::cin };

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