#include "output/formatting.hpp"

namespace tnac_rt::colours
{
#if TNAC_WINDOWS
  //
  // https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
  //

  static void with_colour(std::ostream& out, clr c) noexcept
  {
    using enum clr;
    switch (c)
    {
    case Red:    out << "\x1b[31m"; break;
    case Green:  out << "\x1b[32m"; break;
    case Yellow: out << "\x1b[33m"; break;
    case Blue:   out << "\x1b[34m"; break;
    case White:  out << "\x1b[37m"; break;
    default:                        break;
    }
  }

  void add_clr(std::ostream& out, clr c, bool bold) noexcept
  {
    with_colour(out, c);
    if (bold)
    {
      out << "\x1b[1m";
    }
  }

  void clear_clr(std::ostream& out) noexcept
  {
    out << "\x1b[m";
  }

#else
  static void with_colour(std::ostream& out, clr c) noexcept
  {
    utils::unused(out, c);
  }

  void add_clr(std::ostream& out, clr c, bool bold) noexcept
  {
    utils::unused(out, c, bold);
  }

  void clear_clr(std::ostream& out) noexcept
  {
    utils::unused(out);
  }

#endif
}