//
// IR codes
//

#pragma once

namespace tnac::ir
{
  //
  // Operation code for the IR
  //
  enum class op_code : std::uint8_t
  {
    // Storage access
    Constant, // produce a constant
    Load,     // load a value from memory
    Store,    // store a value into memory

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
    CmpIs      // ?
  };
}