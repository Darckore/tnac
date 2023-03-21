#include "parser/lex.hpp"

void print(const tnac::token& tok) noexcept
{
  static constexpr std::array kinds{
    "Invalid token"sv,
    "Eol"sv,
    "IntDec"sv,
    "Float"sv,
    "Plus"sv,
    "Minus"sv,
    "Asterisk"sv,
    "Slash"sv
  };

  using idx_t = decltype(kinds)::size_type;

  std::cout << kinds[static_cast<idx_t>(tok.m_kind)] << ": '" << tok.m_value << "'\n";
}

bool parse_line(tnac::buf_t input) noexcept
{
  static std::forward_list<tnac::buf_t> lineBuf;
  static tnac::lex lex;

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

    print(tok);
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