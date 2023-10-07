//
// Basic formatting
//

#pragma once

namespace tnac_rt::fmt
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
  // Prints the given message and applies the specified colour to it
  //
  void print(std::ostream& out, clr c, std::string_view msg) noexcept;

  //
  // Prints the given message and applies the specified colour to it
  // Adds a line feed at the end
  //
  void println(std::ostream& out, clr c, std::string_view msg) noexcept;
}