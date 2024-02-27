//
// IR instructions
//

#pragma once
#include "cfg/ir/ir_base.hpp"
#include "eval/value/value.hpp"
#include "eval/types/traits.hpp"

#define TNAC_OPERANDS eval::value,\
function*,\
basic_block*

namespace tnac::ir
{
  class function;
  class basic_block;

  namespace detail
  {
    //
    // Defines a valid operand
    //
    template <typename T>
    concept operand_data = utils::any_same_as<T, TNAC_OPERANDS>;
  }
}

namespace tnac::ir
{
  //
  // Operand of an instruction
  //
  class operand final
  {
  public:
    using data_type = std::variant<TNAC_OPERANDS>;

  public:
    CLASS_SPECIALS_NODEFAULT(operand);

    ~operand() noexcept = default;

    explicit operand(detail::operand_data auto val) noexcept :
      m_value{ val }
    {}

  private:
    data_type m_value;
  };
}

namespace tnac::ir
{
  //
  // Operation code for instructions
  //
  enum class op_code : std::uint8_t
  {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Pow,
    Root,
    And,
    Or,
    Xor,
    CmpE,
    CmpL,
    CmpLE,
    
    Abs,

    Store,
    Load,

    Call,
    Jump,
    Ret
  };

  //
  // Base class for IR instructions
  //
  class instruction :
    public utils::ilist_node<instruction>,
    public node
  {
  public:
    using enum op_code;
    using op_list = std::vector<operand>;

  public:
    CLASS_SPECIALS_NONE(instruction);

    virtual ~instruction() noexcept;

    instruction(basic_block& owner, op_code code) noexcept;

  public:
    //
    // Returns a reference to the parent basic block
    //
    const basic_block& owner_block() const noexcept;

    //
    // Returns a reference to the parent basic block
    //
    basic_block& owner_block() noexcept;

  private:
    basic_block* m_block{};
    op_list m_operands;
    op_code m_opCode;
  };
}