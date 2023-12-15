//
// IR instructions
//

#pragma once
#include "cfg/ir/ir_base.hpp"

namespace tnac::ir
{
  class basic_block;
}

namespace tnac::ir
{
  //
  // Base class for IR instructions
  //
  class instruction : public node
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(instruction);

    virtual ~instruction() noexcept;

  protected:
    instruction(basic_block& owner, kind k) noexcept;

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
  };
}