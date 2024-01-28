//
// IR function
//

#pragma once
#include "cfg/ir/ir_base.hpp"
#include "cfg/ir/ir_basic_block.hpp"

namespace tnac::ir
{
  //
  // Represents IR functions and modules
  //
  class function final : public node
  {
  public:
    using name_t     = string_t;
    using size_type  = std::uint16_t;
    using child_list = std::vector<function*>;
    using block_list = block_container;

  public:
    CLASS_SPECIALS_NONE(function);

    virtual ~function() noexcept;

    function(name_t name, entity_id id, size_type paramCount, block_list blocks) noexcept;

    function(name_t name, entity_id id, size_type paramCount, function& owner, block_list blocks) noexcept;

  protected:
    function(name_t name, entity_id id, size_type paramCount, function* owner, block_list blocks) noexcept;

  public:
    //
    // Returns the function or module name
    //
    name_t name() const noexcept;

    //
    // Returns the function's id
    //
    entity_id id() const noexcept;

    //
    // Returns the number of function's parameters
    //
    size_type param_count() const noexcept;

    //
    // Returns a reference to the entry block
    // Doesn't check whether an entry has been assigned, whoever creates functions
    // is responsible for properly initialising the entry block and must guarantee its existence
    // 
    // const version
    //
    const basic_block& entry() const noexcept;

    //
    // Returns a reference to the entry block
    // Doesn't check whether an entry has been assigned, whoever creates functions
    // is responsible for properly initialising the entry block and must guarantee its existence
    //
    basic_block& entry() noexcept;

    //
    // Returns a pointer to the owner function
    // 
    // const version
    //
    const function* owner_func() const noexcept;

    //
    // Returns a pointer to the owner function
    //
    function* owner_func() noexcept;

    //
    // Returns a list of this function's nested functions
    // 
    // const version
    //
    const child_list& children() const noexcept;

    //
    // Returns a list of this function's nested functions
    //
    child_list& children() noexcept;

    //
    // Returns a reference to the basic block container
    // 
    // const version
    //
    const block_list& blocks() const noexcept;

    //
    // Returns a reference to the basic block container
    //
    block_list& blocks() noexcept;

    //
    // Creates a basic block with the given name
    //
    basic_block& create_block(string_t name) noexcept;

  private:
    //
    // Adds a nested function
    //
    void add_child(function& child) noexcept;

  private:
    name_t m_name;
    function* m_owner{};
    child_list m_children;
    block_list m_blocks;
    basic_block* m_entry{};
    entity_id m_id;
    size_type m_paramCount{};
  };
}