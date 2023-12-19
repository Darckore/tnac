//
// Basic block
//

#pragma once
#include "cfg/ir/ir_base.hpp"

namespace tnac::ir
{
  class function;
}

namespace tnac::ir
{
  //
  // Represents a basic block of the CFG
  //
  class basic_block final : public node
  {
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
    // const version
    //
    const function& func() const noexcept;

    //
    // Returns a reference to the owner function
    //
    function& func() noexcept;

  private:
    function* m_owner{};
    string_t m_name;
  };
}

namespace tnac::ir::detail
{
  //
  // A proxy container for basic blocks to be used in functions
  //
  class block_container final
  {
  public:
    using underlying_t = std::unordered_map<string_t, basic_block>;
    using pointer = underlying_t*;
    using reference = underlying_t&;
    using const_pointer = const underlying_t*;
    using const_reference = const underlying_t&;

  public:
    CLASS_SPECIALS_NODEFAULT(block_container);

    ~block_container() noexcept;
    explicit block_container(reference value) noexcept;

  public:
    //
    // Locates a basic block by name
    // 
    // const version
    //
    const basic_block* find(string_t name) const noexcept;

    //
    // Locates a basic block by name
    //
    basic_block* find(string_t name) noexcept;

    //
    // Removes the specified basic block
    //
    void remove(string_t name) noexcept;

    //
    // Adds a basic block
    //
    basic_block& add(string_t name, function& owner) noexcept;

  private:
    pointer m_value{};
  };
}