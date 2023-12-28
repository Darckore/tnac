//
// IR printer
//

#pragma once
#include "output/common.hpp"

namespace tnac::rt::out
{
  //
  // Printer for IR
  // Visits each function in declaration order its instructions to the specified stream
  //
  class ir_printer : public ir::const_walker<ir_printer>
  {
  public:
    using base = ir::const_walker<ir_printer>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(ir_printer);

    ~ir_printer() noexcept;

    ir_printer() noexcept;

  public:
    void operator()(const ir::cfg& gr, out_stream& os) noexcept;

    void operator()(const ir::cfg& gr) noexcept;

  public:
    void visit(const ir::function& fn) noexcept;

    void visit(const ir::basic_block& bb) noexcept;

    void visit(const ir::instruction& instr) noexcept;

  private:
    out_stream& out() noexcept;

  private:
    out_stream* m_out{ &std::cout };
  };
}