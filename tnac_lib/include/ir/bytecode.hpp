//
// IR codes
//

#pragma once

namespace tnac::ir
{
  //
  // Command code for the IR
  //
  enum class comm_code : std::uint8_t
  {
    // Arithmetic
    Add,  // +
    Sub,  // -
    Mul,  // *
    Div,  // /
    Mod,  // %
    And,  // &
    Xor,  // ^
    Or,   // |
    Pow,  // **
    Root, // //
    Abs,  // | |

    // Logical
    CmpLess,   // <
    CmpLessEq, // <=
    CmpGr,     // >
    CmpGrEq,   // >=
    Eq,        // ==
    NEq,       // !=
    CmpNot,    // !
    CmpIs,     // ?

    // Storage access
    Load,
    Store

  };
}