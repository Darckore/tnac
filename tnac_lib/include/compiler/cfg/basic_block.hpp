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
    CLASS_SPECIALS_NONE(bb_conn);

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
    using conn_ptr      = std::unique_ptr<bb_conn>;
    using out_conn_list = std::vector<conn_ptr>;
    using in_conn_list  = std::vector<bb_conn*>;
    
    using name_t = buf_t;

  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(basic_block);

    ~basic_block() noexcept;
    
    basic_block(name_t name) noexcept;

  public:
    //
    // Returns the name
    //
    string_t name() const noexcept;

    //
    // Creates an outbound connection to another block
    //
    bb_conn& add_outbound(basic_block& target) noexcept;

  private:
    //
    // Adds a pointer to an inbound connection to another block
    //
    void add_inbound(bb_conn& connection) noexcept;

  private:
    name_t m_name;
    out_conn_list m_outbound;
    in_conn_list m_inbound;
  };
}