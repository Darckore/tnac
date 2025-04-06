//
// IR builder
//

#pragma once
#include "cfg/ir/ir.hpp"

namespace tnac::ir
{
  //
  // Creates and manages lifetime of IR nodes
  //
  class builder final
  {
  private:
    struct arr_descr;

  public:
    using fname_t          = function::name_t;
    using par_size_t       = function::size_type;
    using func_store       = std::unordered_map<entity_id, function>;
    using block_map        = block_container::underlying_t;
    using block_store      = std::unordered_map<entity_id, block_map>;
    using instruction_list = instruction::list_type;
    using register_store   = std::forward_list<vreg>;
    using edge_list        = edge::list_type;
    using const_list       = constant::list_type;
    using const_val        = constant::value_type;
    using size_type        = instruction::size_type;
    using arr_store        = std::unordered_map<entity_id, arr_descr>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(builder);

    ~builder() noexcept;

    builder() noexcept;

  public:
    //
    // Creates a module
    //
    function& make_module(entity_id id, fname_t name, par_size_t paramCount) noexcept;

    //
    // Creates a function
    //
    function& make_function(entity_id id, function& owner, fname_t name, par_size_t paramCount) noexcept;

    //
    // Finds a stored function by id
    //
    function* find_function(entity_id id) noexcept;

    //
    // Appends an instruction to the specified basic block before the given iterator
    //
    instruction& add_instruction(basic_block& owner, op_code op, instruction_list::iterator pos) noexcept;

    //
    // Appends an instruction to the specified basic block before the given iterator
    // Preallocs space for operands according to the count parameter
    //
    instruction& add_instruction(basic_block& owner, op_code op, size_type count, instruction_list::iterator pos) noexcept;

    //
    // Adds a variable declaration to the specified block before the given iterator
    //
    instruction& add_var(basic_block& owner, instruction_list::iterator pos) noexcept;

    //
    // Adds an array allocation to the specified block before the given iterator
    //
    instruction& add_array(basic_block& owner, instruction_list::iterator pos) noexcept;

    //
    // Creates a new named virtual register
    //
    vreg& make_register(string_t name) noexcept;

    //
    // Creates a new nameless virtual register
    //
    vreg& make_register(vreg::idx_type idx) noexcept;

    //
    // Creates a new named global register
    //
    vreg& make_global_register(string_t name) noexcept;

    //
    // Creates a new nameless global register
    //
    vreg& make_global_register(vreg::idx_type idx) noexcept;

    //
    // Creates an edge between basic blocks
    //
    edge& make_edge(basic_block& from, basic_block& to, operand val) noexcept;

    //
    // Interns a global array
    //
    constant& intern(vreg& reg, eval::array_type val) noexcept;

    //
    // Returns a reference to the instruction list
    //
    instruction_list& instructions() noexcept;

    //
    // Returns a reference to the edge list
    //
    edge_list& edges() noexcept;

    //
    // Returns a reference to the list of interned values
    //
    const_list& interned() noexcept;

    //
    // Returns an interned constant by array instance
    //
    constant* interned(const eval::array_type& val) noexcept;

  private:
    //
    // Creates a generic function
    //
    function& make_function(entity_id id, function* owner, fname_t name, par_size_t paramCount) noexcept;

    //
    // Creates a generic allocation
    //
    instruction& add_alloc(basic_block& owner, op_code oc, instruction_list::iterator pos) noexcept;

  private:
    func_store m_functions;
    block_store m_blocks;
    instruction_list m_instructions;
    edge_list m_edges;
    const_list m_consts;
    register_store m_regs;
    arr_store m_arrays;
  };
}