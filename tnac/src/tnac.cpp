#include "parser/lex.hpp"

bool parse_line(tnac::buf_t input) noexcept
{
  static tnac::lex lex;
  lex.feed(std::move(input));

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
  }

  return 0;
}