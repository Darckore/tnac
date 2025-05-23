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

    void visit(const ir::constant& val) noexcept;

  private:
    void print_preds(const ir::basic_block& target) noexcept;

    void print_operand(const ir::operand& op) noexcept;

    void print_assign(const ir::operand& op) noexcept;

    void print_alloc(const ir::instruction& alloc) noexcept;

    void print_store(const ir::instruction& store) noexcept;

    void print_append(const ir::instruction& append) noexcept;

    void print_load(const ir::instruction& load) noexcept;

    void print_call(const ir::instruction& call) noexcept;

    void print_binary(const ir::instruction& bin) noexcept;

    void print_unary(const ir::instruction& un) noexcept;

    void print_select(const ir::instruction& sel) noexcept;

    void print_ret(const ir::instruction& ret) noexcept;

    void print_jump(const ir::instruction& jmp) noexcept;

    void print_phi(const ir::instruction& phi) noexcept;

    void print_inst(const ir::instruction& inst) noexcept;

    void print_dyn_bind(const ir::instruction& dyn) noexcept;

  private:
    out_stream& out() noexcept;

    void endl() noexcept;

    void kw_string(string_t kw) noexcept;

    void keyword(string_t kw, bool addSpace = true) noexcept;

    void name(string_t n) noexcept;

    void string_op(string_t str) noexcept;

    void id(entity_id i) noexcept;

    void id(eval::type_id i) noexcept;

    void value(const eval::value& val, bool refInterned = true) noexcept;

    void idx(ir::operand::idx_type i) noexcept;

    void vreg(const ir::vreg& reg) noexcept;

    void param(ir::func_param par) noexcept;

    void block(const ir::basic_block& block) noexcept;

    void edge(const ir::edge& edge) noexcept;

    void plain(string_t str) noexcept;

    void declare_funcs() noexcept;

    void declare(const ir::function& fn) noexcept;

    void func_intro(const ir::function& fn) noexcept;

  private:
    const ir::cfg* m_cfg{};
    out_stream* m_out{ &std::cout };
  };
}