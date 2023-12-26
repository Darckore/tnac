//
// IR instructions
//

#pragma once
#include "cfg/ir/ir_base.hpp"

namespace tnac::ir
{
  //
  // Operation code for instructions
  //
  enum class op_code : std::uint8_t
  {
    Arithmetic,
    Store,
    Load,
    Call,
    Jump,
    Ret
  };

  //
  // Base class for IR instructions
  //
  class instruction : public node
  {
  public:
    using enum op_code;

  public:
    CLASS_SPECIALS_NONE(instruction);

    virtual ~instruction() noexcept;

    instruction(basic_block& owner, op_code code) noexcept;

  public:
    //
    // Returns a reference to the parent basic block
    // 
    // const version
    //
    const basic_block& owner_block() const noexcept;

    //
    // Returns a reference to the parent basic block
    //
    basic_block& owner_block() noexcept;

  private:
    basic_block* m_block{};
    op_code m_opCode;
  };
}