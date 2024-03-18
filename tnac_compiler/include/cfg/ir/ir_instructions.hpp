//
// IR instructions
//

#pragma once
#include "cfg/ir/ir_base.hpp"
#include "eval/value/value.hpp"
#include "eval/types/traits.hpp"

#define TNAC_OPERANDS eval::value,\
function*,\
basic_block*,\
vreg*

namespace tnac::ir
{
  //
  // Virtual register to be used in operands
  //
  class vreg : public node
  {
  public:
    using idx_type = std::uint64_t;
    using id_type  = std::variant<string_t, idx_type>;

  public:
    CLASS_SPECIALS_NONE(vreg);

    ~vreg() noexcept;

    explicit vreg(string_t name) noexcept;
    
    explicit vreg(idx_type idx) noexcept;

  private:
    struct from_id {};
    vreg(from_id, id_type id) noexcept;

  public:
    //
    // Checks whether the register has a name
    //
    bool is_named() const noexcept;

    //
    // Obtains the register's name
    // Callers must ensure that is_named returns true
    //
    string_t name() const noexcept;

    //
    // Obtains the register's index
    // Callers must ensure that is_named returns false
    //
    idx_type index() const noexcept;

  private:
    id_type m_id;
  };
}

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

    ~operand() noexcept;

    explicit operand(detail::operand_data auto val) noexcept :
      m_value{ val }
    {}

  public:
    //
    // Checks whether the operand holds a known value
    //
    bool is_value() const noexcept;

    //
    // Checks whether the operand holds a register reference
    //
    bool is_register() const noexcept;

    //
    // Returns the stored value
    // Callers must check is_value before using this
    //
    eval::value get_value() const noexcept;

    //
    // Returns the stored register
    // Callers must check is_register before using this
    //
    vreg& get_reg() const noexcept;

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
    Alloc,

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
    using op_list   = std::vector<operand>;
    using size_type = op_list::size_type;

  public:
    CLASS_SPECIALS_NONE(instruction);

    virtual ~instruction() noexcept;

    instruction(basic_block& owner, op_code code) noexcept;

    //
    // Returns the operand at the specified index
    // DOES NOT check the boundaries
    //
    const operand& operator[](size_type idx) const noexcept;

  public:
    //
    // Returns the opcode
    //
    op_code opcode() const noexcept;

    //
    // Returns a reference to the parent basic block
    //
    const basic_block& owner_block() const noexcept;

    //
    // Returns a reference to the parent basic block
    //
    basic_block& owner_block() noexcept;

    //
    // Adds an operand to the instruction
    //
    instruction& add(operand op) noexcept;

    //
    // Returns the number of operands
    //
    size_type operand_count() const noexcept;

    //
    // Instruction opcode as a string
    //
    string_t opcode_str() const noexcept;

  private:
    //
    // Reserves memory for operands according to the op code
    //
    void prealloc() noexcept;

  private:
    basic_block* m_block{};
    op_list m_operands;
    op_code m_opCode;
  };
}