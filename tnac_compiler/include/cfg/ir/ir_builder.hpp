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
  public:
    using fname_t          = function::name_t;
    using par_size_t       = function::size_type;
    using func_store       = std::unordered_map<entity_id, function>;
    using block_map        = block_container::underlying_t;
    using block_store      = std::unordered_map<entity_id, block_map>;
    using instruction_list = instruction::list_type;

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
    // Appends an instruction to the specified basic block
    //
    instruction& add_instruction(basic_block& owner, op_code op) noexcept;

  private:
    //
    // Creates a generic function
    //
    function& make_function(entity_id id, function* owner, fname_t name, par_size_t paramCount) noexcept;

  private:
    func_store m_functions;
    block_store m_blocks;
    instruction_list m_instructions;
  };
}