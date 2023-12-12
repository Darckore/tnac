//
// IR base
//

#pragma once

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

  public:
    CLASS_SPECIALS_NONE(instruction);

    virtual ~instruction() noexcept;

    explicit instruction(code c) noexcept;

  public:
    //
    // Returns the instruction code
    //
    code what() const noexcept;

  private:
    code m_code{};
  };

  inline auto get_id(const instruction& i) noexcept
  {
    return i.what();
  }
}