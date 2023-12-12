//
// IR base
//

#pragma once

namespace tnac::ir
{
  class basic_block;
}

namespace tnac::ir
{
  //
  // Instruction codes
  //
  enum class instruction_code : std::uint8_t
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
  class instruction
  {
  public:
    using code = instruction_code;
    using enum instruction_code;

  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(instruction);

    virtual ~instruction() noexcept;

  protected:
    instruction(basic_block& owner, code c) noexcept;

  public:
    //
    // Returns the instruction code
    //
    code what() const noexcept;

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
    code m_code{};
  };

  inline auto get_id(const instruction& i) noexcept
  {
    return i.what();
  }
}