#include "output/formatting.hpp"

namespace tnac_rt::colours
{
#if TNAC_WINDOWS
  //
  // https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
  //

  void add_clr(std::ostream& out, clr c) noexcept
  {
    using enum clr;
    switch (c)
    {
    case Red:        out << "\x1b[31m";        break;
    case Green:      out << "\x1b[32m";        break;
    case Yellow:     out << "\x1b[33m";        break;
    case Blue:       out << "\x1b[34m";        break;
    case Cyan:       out << "\x1b[36m";        break;
    case White:      out << "\x1b[37m";        break;
    case BoldRed:    out << "\x1b[31m";        break;
    case BoldGreen:  out << "\x1b[32m\x1b[1m"; break;
    case BoldYellow: out << "\x1b[33m\x1b[1m"; break;
    case BoldBlue:   out << "\x1b[34m\x1b[1m"; break;
    case BoldCyan:   out << "\x1b[36m\x1b[1m"; break;
    case BoldWhite:  out << "\x1b[37m\x1b[1m"; break;
    default:                                   break;
    }
  }

  void clear_clr(std::ostream& out) noexcept
  {
    out << "\x1b[m";
  }

#else
  void add_clr(std::ostream& out, clr c) noexcept
  {
    utils::unused(out, c);
  }

  void clear_clr(std::ostream& out) noexcept
  {
    utils::unused(out);
  }
#endif

  void print(std::ostream& out, clr c, std::string_view msg) noexcept
  {
    // Styling only makes sence for std::cout and std::cerr
    if (utils::eq_none(&out, &std::cout, &std::cerr))
    {
      out << msg;
      return;
    }

    add_clr(out, c);
    out << msg;
    clear_clr(out);
  }

  void println(std::ostream& out, clr c, std::string_view msg) noexcept
  {
    print(out, c, msg);
    out << '\n';
  }
}