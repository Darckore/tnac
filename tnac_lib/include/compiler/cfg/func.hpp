//
// Cfg function
//

#pragma once
#include "compiler/cfg/basic_block.hpp"
#include "compiler/cfg/environment.hpp"

namespace tnac::comp
{
  //
  // Represents a function in the cfg
  //
  class func final
  {
  public:
    using name_t = buf_t;
    using storage_key = utils::hashed_string;
    using block_store = std::unordered_map<storage_key, basic_block>;

  public:
    CLASS_SPECIALS_NONE(func);

    ~func() noexcept;

    func(name_t name, func* parent) noexcept;

  public:
    //
    // Returns the function's mangled name
    //
    string_t name() const noexcept;

    //
    // Returns the function's parent (can be null)
    // 
    // const version
    //
    const func* parent() const noexcept;

    //
    // Returns the function's parent (can be null)
    //
    func* parent() noexcept;

    //
    // Creates a basic block
    //
    basic_block& create_block(name_t name) noexcept;

    //
    // Enters a basic block
    //
    void enter_block(basic_block& block) noexcept;

    //
    // Returns a reference to the entry basic block
    // 
    // const version
    //
    const basic_block& entry() const noexcept;

    //
    // Returns a reference to the entry basic block
    //
    basic_block& entry() noexcept;

    //
    // Returns a reference to the current basic block
    // 
    // const version
    //
    const basic_block& current_block() const noexcept;

    //
    // Returns a reference to the current basic block
    //
    basic_block& current_block() noexcept;

    //
    // Returns a reference to the environment
    // 
    // const version
    //
    const environment& env() const noexcept;

    //
    // Returns a reference to the environment
    //
    environment& env() noexcept;

  private:
    name_t m_name;
    func* m_parent{};
    block_store m_blocks;
    basic_block* m_entry{};
    basic_block* m_cur{};
    environment m_env;
  };
}