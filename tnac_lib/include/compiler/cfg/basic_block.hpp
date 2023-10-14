//
// Basic block
//

#pragma once
#include "compiler/cfg/edge.hpp"
#include "ir/ir.hpp"

namespace tnac::comp
{
  class func;

  //
  // Basic block of the cfg
  //
  class basic_block final
  {
  public:
    using edge_ptr      = std::unique_ptr<cfg_edge>;
    using out_edge_list = std::vector<edge_ptr>;
    using in_edge_list  = std::vector<cfg_edge*>;
    using name_t = buf_t;

    using op_t = ir::operation;
    using op_store = std::vector<op_t>;
    using size_type = op_store::size_type;

  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(basic_block);

    ~basic_block() noexcept;

    basic_block(name_t name, func& parent) noexcept;

  public:
    //
    // Returns the name
    //
    string_t name() const noexcept;

    //
    // Returns the parent function
    // 
    // const version
    //
    const func& parent() const noexcept;

    //
    // Returns the parent function
    //
    func& parent() noexcept;

    //
    // Creates an outbound connection to another block
    //
    cfg_edge& add_outbound(basic_block& target) noexcept;

    //
    // Adds an operation
    //
    op_t& add_operation(ir::op_code op, op_t::size_type size) noexcept;

  private:
    //
    // Adds a pointer to an inbound connection to another block
    //
    void add_inbound(cfg_edge& connection) noexcept;

  private:
    name_t m_name;
    func* m_parent{};
    out_edge_list m_outbound;
    in_edge_list m_inbound;
    op_store m_operations;
  };
}