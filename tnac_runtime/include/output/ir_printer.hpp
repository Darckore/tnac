//
// IR printer
//

#pragma once
#include "output/common.hpp"

namespace tnac::rt::out
{
  //
  // Printer for IR
  // Visits each function in declaration order
  // and prints its instructions to the specified stream
  //
  class ir_printer : public ir::const_walker<ir_printer>
  {
  public:
    using base = ir::const_walker<ir_printer>;
    using param_idx = std::uint16_t;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(ir_printer);

    ~ir_printer() noexcept;

    ir_printer() noexcept;

  public:
    void operator()(const ir::cfg& gr, out_stream& os) noexcept;

    void operator()(const ir::cfg& gr) noexcept;

  public:
    bool preview(const ir::function& fn) noexcept;
    
    bool preview(const ir::basic_block& bb) noexcept;

    void visit(const ir::function& fn) noexcept;

    void visit(const ir::basic_block& bb) noexcept;

    void visit(const ir::instruction& instr) noexcept;

  private:
    void print_operand(const ir::operand& op) noexcept;

    void print_assign(const ir::operand& op) noexcept;

    void print_alloc(const ir::instruction& alloc) noexcept;

    void print_ret(const ir::instruction& ret) noexcept;

  private:
    out_stream& out() noexcept;

    void endl() noexcept;

    void keyword(string_t kw) noexcept;

    void name(string_t n) noexcept;

    void id(entity_id i) noexcept;

    void value(eval::value val) noexcept;

    void vreg(ir::vreg& reg) noexcept;

    void plain(string_t str) noexcept;

    void declare_funcs(const ir::cfg& gr) noexcept;

    void declare(const ir::function& fn) noexcept;

    void func_intro(const ir::function& fn) noexcept;

  private:
    out_stream* m_out{ &std::cout };
  };
}