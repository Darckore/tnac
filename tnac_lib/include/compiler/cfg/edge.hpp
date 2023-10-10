//
// Edge
//

#pragma once

namespace tnac::comp
{
  class basic_block;

  //
  // CFG edge connecting basic blocks
  //
  class cfg_edge final
  {
  public:
    CLASS_SPECIALS_NONE(cfg_edge);

    ~cfg_edge() noexcept;

    cfg_edge(basic_block& from, basic_block& to) noexcept;

  public:
    //
    // Returns a reference to the source block
    //
    basic_block& source() noexcept;

    //
    // Returns a reference to the destination block
    //
    basic_block& dest() noexcept;

  private:
    basic_block* m_from{};
    basic_block* m_to{};
  };
}