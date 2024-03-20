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
  class edge final :
    public utils::ilist_node<edge>,
    public node
  {
  public:
    CLASS_SPECIALS_NONE(edge);

    ~edge() noexcept;

    edge(basic_block& in, basic_block& out, operand val) noexcept;

  public:
    //
    // Returns a reference to the incoming basic block
    //
    const basic_block& incoming() const noexcept;

    //
    // Returns a reference to the incoming basic block
    //
    basic_block& incoming() noexcept;

    //
    // Returns a reference to the outgoing basic block
    //
    const basic_block& outgoing() const noexcept;

    //
    // Returns a reference to the outgoing basic block
    //
    basic_block& outgoing() noexcept;

    //
    // Returns the attached value
    //
    operand value() const noexcept;

  private:
    basic_block* m_in{};
    basic_block* m_out{};
    operand m_value;
  };


  //
  // Represents a basic block of the CFG
  //
  class basic_block final : public node
  {
  public:
    using instruction_iter       = utils::ilist<instruction>::iterator;
    using const_instruction_iter = utils::ilist<instruction>::const_iterator;
    using edge_list              = std::vector<edge*>;
    using edge_view              = std::span<edge*>;
    using const_edge_view        = std::span<const edge* const>;

  protected:
    friend class edge;

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
    // Deletes all instructions and sets first and last iterators to nothing
    //
    void clear_instructions() noexcept;

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

    //
    // Checks whether the given edge is the last pred
    //
    bool is_last_pred(const edge& e) const noexcept;

    //
    // Checks whether the last pred connects to the given block
    //
    bool is_last_connection(const basic_block& bb) const noexcept;

    //
    // Returns a view into preds
    //
    const_edge_view preds() const noexcept;

    //
    // Returns a view into preds
    //
    edge_view preds() noexcept;

    //
    // Returns a view into outs
    //
    const_edge_view outs() const noexcept;

    //
    // Returns a view into outs
    //
    edge_view outs() noexcept;

  protected:
    //
    // Appends an incoming edge
    //
    void add_pred(edge* e) noexcept;

    //
    // Appends an outgoind edge
    //
    void add_out(edge* e) noexcept;

  private:
    function* m_owner{};
    string_t m_name;
    instruction_iter m_first;
    instruction_iter m_last;
    edge_list m_in;
    edge_list m_out;
  };

  using block_container = detail::ir_container<string_t, basic_block>;
}