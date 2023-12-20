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
  // An iterator for a collection of basic blocks
  //
  class block_iterator final
  {
  public:
    using underlying_t = std::unordered_map<string_t, basic_block>;
    using iter         = underlying_t::iterator;
    using size_type    = underlying_t::size_type;
    friend class block_container;

  public:
    CLASS_SPECIALS_NODEFAULT(block_iterator);

    ~block_iterator() noexcept = default;

    explicit block_iterator(iter it) noexcept :
      m_iter{ it }
    {}

    bool operator==(const block_iterator&) const noexcept = default;

    auto operator++() noexcept
    {
      ++m_iter;
      return *this;
    }
    auto operator++(int) noexcept
    {
      auto self = *this;
      ++(*this);
      return self;
    }

    basic_block& operator*() noexcept
    {
      return m_iter->second;
    }

    auto operator->() noexcept
    {
      return &(operator*());
    }

    auto key() const noexcept
    {
      return m_iter->first;
    }

  protected:
    iter get() noexcept
    {
      return m_iter;
    }

  private:
    iter m_iter;
  };

  //
  // A proxy container for basic blocks to be used in functions
  //
  class block_container final
  {
  public:
    using underlying_t    = block_iterator::underlying_t;
    using pointer         = underlying_t*;
    using reference       = underlying_t&;
    using const_pointer   = const underlying_t*;
    using const_reference = const underlying_t&;

  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(block_container);

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
    // Removes the block pointed to by an iterator
    //
    void remove(block_iterator it) noexcept;

    //
    // Adds a basic block
    //
    basic_block& add(string_t name, function& owner) noexcept;

  public:
    auto begin() const noexcept
    {
      return block_iterator{ m_value->begin() };
    }
    auto begin() noexcept
    {
      return block_iterator{ m_value->begin() };
    }
    auto end() const noexcept
    {
      return block_iterator{ m_value->end() };
    }
    auto end() noexcept
    {
      return block_iterator{ m_value->end() };
    }

  private:
    pointer m_value{};
  };
}