//
// Basic block
//

#pragma once
#include "compiler/cfg/edge.hpp"
#include "compiler/cfg/environment.hpp"
#include "ir/ir_all.hpp"

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
    using name_t        = buf_t;
    using reg_index_t   = environment::reg_index_t;

    using byte_t    = std::byte;
    using op_store  = std::vector<byte_t>;
    using size_type = op_store::size_type;

  public:
    CLASS_SPECIALS_NONE(basic_block);

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

  public: // Operations
    //
    // Adds a constant
    //
    void add_constant(reg_index_t saveTo, eval::value val) noexcept;

    //
    // Adds a unary operation
    //
    void add_unary(ir::op_code code, reg_index_t saveTo, reg_index_t operand) noexcept;

    //
    // Adds a binary operation
    //
    void add_binary(ir::op_code code, reg_index_t saveTo, reg_index_t left, reg_index_t right) noexcept;

  private:
    //
    // Adds a pointer to an inbound connection to another block
    //
    void add_inbound(cfg_edge& connection) noexcept;

    //
    // Preallocates memory for an operation
    //
    void alloc_op(size_type size) noexcept;

    //
    // Returns the size of the target value plus type id
    //
    size_type value_size(eval::value val) const noexcept;

    //
    // Adds an operand
    //
    template <typename T>
    void add_operand(T val) noexcept
    {
      constexpr auto dataSize = sizeof(val);
      std::array<byte_t, dataSize> byteArr{};
      std::memcpy(byteArr.data(), &val, dataSize);
      m_opData.append_range(byteArr);
    }

    //
    // Adds a value operand
    //
    void add_value(eval::value val) noexcept;

    //
    // Adds a virtula register index to store the result in
    //
    void add_register(reg_index_t idx) noexcept;

    //
    // Adds an bool operand
    //
    void add_value(eval::bool_type val) noexcept;

    //
    // Adds an int operand
    //
    void add_value(eval::int_type val) noexcept;

    //
    // Adds a float operand
    //
    void add_value(eval::float_type val) noexcept;

    //
    // Adds a complex operand
    //
    void add_value(eval::complex_type val) noexcept;

    //
    // Adds a type id prefix to the operand
    //
    void add_type_id(eval::type_id ti) noexcept;

  private:
    name_t m_name;
    func* m_parent{};
    out_edge_list m_outbound;
    in_edge_list m_inbound;
    op_store m_opData;
  };
}