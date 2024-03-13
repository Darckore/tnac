//
// Basic block
//

#pragma once
#include "cfg/ir/ir_base.hpp"
#include "cfg/ir/ir_instructions.hpp"

namespace tnac::ir
{
  class function;
  class basic_block;
}

namespace tnac::ir
{
  //
  // Represents a graph edge connecting two basic blocks
  //
  class edge final : public node
  {
  public:
    CLASS_SPECIALS_NONE(edge);
    ~edge() noexcept;

  protected:
    friend class basic_block;
    edge(basic_block& in, basic_block& out) noexcept;

  public:
    //
    // Returns a reference to the incoming basic block
    // 
    // const vesrion
    //
    const basic_block& incoming() const noexcept;

    //
    // Returns a reference to the incoming basic block
    //
    basic_block& incoming() noexcept;

    //
    // Returns a reference to the outgoing basic block
    // 
    // const vesrion
    //
    const basic_block& outgoing() const noexcept;

    //
    // Returns a reference to the outgoing basic block
    //
    basic_block& outgoing() noexcept;

  private:
    basic_block* m_in{};
    basic_block* m_out{};
  };


  //
  // Represents a basic block of the CFG
  //
  class basic_block final : public node
  {
  public:
    using instruction_iter = utils::ilist<instruction>::iterator;
    using const_instruction_iter = utils::ilist<instruction>::const_iterator;

  public:
    CLASS_SPECIALS_NONE(basic_block);

    virtual ~basic_block() noexcept;

    basic_block(string_t name, function& owner) noexcept;

  public:
    //
    // Returns the block name
    //
    string_t name() const noexcept;

    //
    // Returns a reference to the owner function
    //
    const function& func() const noexcept;

    //
    // Returns a reference to the owner function
    //
    function& func() noexcept;

    //
    // Appends an instruction
    //
    basic_block& add_instruction(instruction& in) noexcept;

    //
    // Prepends an instruction
    //
    basic_block& add_instruction_front(instruction& in) noexcept;

    //
    // Returns an iterator to the first instruction
    //
    instruction_iter begin() noexcept;

    //
    // Returns an iterator to the first instruction
    //
    const_instruction_iter begin() const noexcept;

    //
    // Returns an iterator past the last instruction
    //
    instruction_iter end() noexcept;

    //
    // Returns an iterator past the last instruction
    //
    const_instruction_iter end() const noexcept;

  private:
    function* m_owner{};
    string_t m_name;
    instruction_iter m_first;
    instruction_iter m_last;
  };

  using block_container = detail::ir_container<string_t, basic_block>;
}