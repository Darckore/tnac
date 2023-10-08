//
// Basic block
//

#pragma once

namespace tnac::comp
{
  class basic_block;

  //
  // Connection between basic blocks
  //
  class bb_conn final
  {
  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(bb_conn);

    ~bb_conn() noexcept;

    bb_conn(basic_block& from, basic_block& to) noexcept;

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


  //
  // Basic block of the cfg
  //
  class basic_block final
  {
  public:
    using conn_list = std::vector<bb_conn>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(basic_block);

    ~basic_block() noexcept;
    basic_block() noexcept;

  public:
    //
    // Creates an outbound connection to another block
    //
    bb_conn& add_outbound(basic_block& target) noexcept;

  private:
    conn_list m_outbound;
  };
}