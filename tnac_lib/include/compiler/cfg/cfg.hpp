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
    using stored_block = std::unique_ptr<basic_block>;
    using block_store  = std::vector<stored_block>;

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
    // Returns a reference to the entry block
    //
    basic_block& entry() noexcept;

  private:
    block_store m_blocks;
  };
}