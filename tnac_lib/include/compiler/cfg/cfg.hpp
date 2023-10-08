//
// CFG
//

#pragma once
#include "compiler/cfg/basic_block.hpp"

namespace tnac::comp
{
  //
  // Control-flow graph for the program
  //
  class cfg final
  {
  public:
    using storage_key = utils::hashed_string;
    using block_name  = basic_block::name_t;
    using block_store = std::unordered_map<storage_key, basic_block>;
    using entry_stack = std::stack<basic_block*>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(cfg);

    ~cfg() noexcept;
    cfg() noexcept;

    //
    // Returns true if the cfg has an entry point
    // (i.e., is not empty)
    //
    explicit operator bool() const noexcept;

  public:
    //
    // Enters a basic block
    //
    void enter_block(basic_block& block) noexcept;

    //
    // Exits a basic block and rolls back to the one
    // entered before
    //
    void exit_block() noexcept;

    //
    // Creates a new basic block
    //
    basic_block& create(block_name name) noexcept;

    //
    // Returns a reference to the entry block
    //
    basic_block& entry() noexcept;

    //
    // Attempts to find a basic block by name
    //
    basic_block* find(storage_key name) noexcept;

  private:
    basic_block* m_entry{};
    block_store m_blocks;
    entry_stack m_entryChain;
  };
}