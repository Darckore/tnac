//
// Basic formatting
//

#pragma once

namespace tnac_rt::colours
{
  enum class clr : std::uint8_t
  {
    Default,
    Red,
    Yellow,
    Green,
    Blue,
    White
  };

  //
  // Applies the specified colour and, optionally,
  // makes the printed text bold
  //
  void add_clr(std::ostream& out, clr c, bool bold) noexcept;

  //
  // Reverts text style to default
  //
  void clear_clr(std::ostream& out) noexcept;
}