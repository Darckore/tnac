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
    using block_list = detail::block_container;

  public:
    CLASS_SPECIALS_NONE(function);

    virtual ~function() noexcept;

    function(name_t name, size_type paramCount, block_list blocks) noexcept;

    function(name_t name, size_type paramCount, function& owner, block_list blocks) noexcept;

  protected:
    function(name_t name, size_type paramCount, function* owner, block_list blocks) noexcept;

  public:
    //
    // Returns the function or module name
    //
    name_t name() const noexcept;

    //
    // Returns the number of function's parameters
    //
    size_type param_count() const noexcept;

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
    size_type m_paramCount{};
  };
}