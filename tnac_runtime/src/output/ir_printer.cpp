#include "output/ir_printer.hpp"

namespace tnac::rt::out
{
  // Special members

  ir_printer::~ir_printer() noexcept = default;

  ir_printer::ir_printer() noexcept = default;


  // Public members

  void ir_printer::operator()(const ir::cfg& gr, out_stream& os) noexcept
  {
    m_out = &os;
    base::operator()(gr);
  }

  void ir_printer::operator()(const ir::cfg& gr) noexcept
  {
    operator()(gr, out());
  }

  void ir_printer::visit(const ir::function& fn) noexcept
  {
    out() << fn.name() << '\n';
  }

  void ir_printer::visit(const ir::basic_block& bb) noexcept
  {
    utils::unused(bb);
  }

  void ir_printer::visit(const ir::instruction& instr) noexcept
  {
    utils::unused(instr);
  }


  // Private members

  out_stream& ir_printer::out() noexcept
  {
    return *m_out;
  }

}