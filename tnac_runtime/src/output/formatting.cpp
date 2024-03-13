#include "output/formatting.hpp"
#include "sema/sym/symbols.hpp"
#include "cfg/ir/ir.hpp"
#include "eval/value/value.hpp"
#include "src_mgr/source_location.hpp"

namespace tnac::rt::fmt
{
  void add_clr(std::ostream& out, clr c) noexcept
  {
    if (utils::eq_none(&out, &std::cout, &std::cerr))
      return;

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

tnac::rt::out_stream& operator<<(tnac::rt::out_stream& out, tnac::eval::invalid_val_t) noexcept
{
  out << "<undef>";
  return out;
}

tnac::rt::out_stream& operator<<(tnac::rt::out_stream& out, const tnac::eval::complex_type& c) noexcept
{
  const auto r = c.real();
  const auto i = c.imag();

  out << '(';
  out << r;
  auto sign = (i > 0) ? '+' : '-';
  out << ' ' << sign << ' ' << utils::abs(i) << "i";
  out << ')';
  return out;
}

tnac::rt::out_stream& operator<<(tnac::rt::out_stream& out, const tnac::eval::fraction_type& f) noexcept
{
  if (f.sign() < 0)
    out << '-';

  if (f.is_infinity())
  {
    out << std::numeric_limits<tnac::eval::float_type>::infinity();
    return out;
  }

  const auto den = f.denom();
  auto num = f.num();
  if (den == tnac::eval::int_type{ 1 })
  {
    out << num;
    return out;
  }

  auto wholePart = num / den;

  if (wholePart)
  {
    out << wholePart;
    num = num % den;
    if (!num)
      return out;

    out << '(';
  }

  out << num << '/' << den;

  if (wholePart)
    out << ')';

  return out;
}

tnac::rt::out_stream& operator<<(tnac::rt::out_stream& out, const tnac::eval::function_type& f) noexcept
{
  out << "function: " << f->name() << "( " << f->param_count() << " )";

  return out;
}

tnac::rt::out_stream& operator<<(tnac::rt::out_stream& out, tnac::entity_id id) noexcept
{
  std::print(out, "{:X}"sv, *id);
  return out;
}