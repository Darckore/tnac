#include "output/formatting.hpp"
#include "src_mgr/source_location.hpp"

namespace tnac::rt::fmt
{
  void add_clr(std::ostream& out, clr c) noexcept
  {
    if (utils::eq_none(&out, &std::cout, &std::cerr))
      return;

    using enum clr;
    out << "\x1b[";
    switch (c)
    {
    case Default:     out << "39"; break;
    case Black:       out << "30"; break;
    case White:       out << "97"; break;

    case DarkRed:     out << "31"; break;
    case DarkGreen:   out << "32"; break;
    case DarkYellow:  out << "33"; break;
    case DarkBlue:    out << "34"; break;
    case DarkMagenta: out << "35"; break;
    case DarkCyan:    out << "36"; break;

    case LightGray:   out << "37"; break;
    case DarkGray:    out << "90"; break;
    case Red:         out << "91"; break;
    case Green:       out << "92"; break;
    case Yellow:      out << "93"; break;
    case Blue:        out << "94"; break;
    case Magenta:     out << "95"; break;
    case Cyan:        out << "96"; break;
    }
    out << 'm';
  }

  void clear_clr(std::ostream& out) noexcept
  {
    if (utils::eq_none(&out, &std::cout, &std::cerr))
      return;

    out << "\x1b[m";
  }
}

tnac::rt::out_stream& operator<<(tnac::rt::out_stream & out, const tnac::token& tok) noexcept
{
  out << tok.value();
  return out;
}

tnac::rt::out_stream& operator<<(tnac::rt::out_stream& out, tnac::src::loc_wrapper loc) noexcept
{
  out << '<';
  if (loc)
    out << loc->file().string();
  else
    out << "Unknown"sv;

  out << ">:" << (loc->line() + 1) << ':' << (loc->col() + 1);
  return out;
}

tnac::rt::out_stream& operator<<(tnac::rt::out_stream& out, tnac::entity_id id) noexcept
{
  std::print(out, "{:X}"sv, *id);
  return out;
}