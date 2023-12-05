//
// Basic formatting
//

#pragma once
#include "output/common.hpp"
#include "eval/types/types.hpp"

namespace tnac::rt::fmt::detail
{
  //
  // Checks whether the given type is printable via an ostream
  //
  template <typename T>
  concept printable = requires(out_stream& os, T&& val)
  {
    os << val;
  };
}

namespace tnac::rt::fmt
{
  enum class clr : std::uint8_t
  {
    Default,
    
    Red,
    Yellow,
    Green,
    Blue,
    Cyan,
    White,

    BoldRed,
    BoldYellow,
    BoldGreen,
    BoldBlue,
    BoldCyan,
    BoldWhite
  };

  //
  // Applies the specified colour
  //
  void add_clr(std::ostream& out, clr c) noexcept;

  //
  // Reverts text style to default
  //
  void clear_clr(std::ostream& out) noexcept;

  //
  // Prints the given printable value and applies the specified colour to it
  //
  template <detail::printable P>
  void print(std::ostream& out, clr c, P&& msg) noexcept
  {
    add_clr(out, c);
    out << msg;
    clear_clr(out);
  }

  //
  // Prints the given printable value and applies the specified colour to it
  // Adds a line feed at the end
  //
  template <detail::printable P>
  void println(std::ostream& out, clr c, P&& msg) noexcept
  {
    print(out, c, std::forward<P>(msg));
    out << '\n';
  }
}

//
// Token output
//
tnac::rt::out_stream& operator<<(tnac::rt::out_stream& out, const tnac::token& tok) noexcept;

//
// Location output
//
tnac::rt::out_stream& operator<<(tnac::rt::out_stream& out, tnac::src::loc_wrapper loc) noexcept;

//
// Invalid value output
//
tnac::rt::out_stream& operator<<(tnac::rt::out_stream& out, tnac::eval::invalid_val_t) noexcept;

//
// Compex output
//
tnac::rt::out_stream& operator<<(tnac::rt::out_stream& out, const tnac::eval::complex_type& c) noexcept;

//
// Fraction output
//
tnac::rt::out_stream& operator<<(tnac::rt::out_stream& out, const tnac::eval::fraction_type& f) noexcept;

//
// Function output
//
tnac::rt::out_stream& operator<<(tnac::rt::out_stream& out, const tnac::eval::function_type& f) noexcept;