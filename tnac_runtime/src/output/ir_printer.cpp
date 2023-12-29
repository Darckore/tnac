#include "output/ir_printer.hpp"
#include "output/formatting.hpp"

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

  bool ir_printer::preview(const ir::function& fn) noexcept
  {
    if (fn.owner_func())
      keyword("function "sv);
    else
      keyword("module "sv);

    id(fn.id());
    out() << ' ';
    name(fn.name());
    out() << " (";

    {
      const auto paramCnt = fn.param_count();
      for (auto pi = param_idx{}; pi < paramCnt; ++pi)
      {
        param(pi);
        if (pi + 1 != paramCnt)
          out() << ", ";
      }
    }

    out() << ')' << '\n';

    endl();
    return true;
  }

  void ir_printer::visit(const ir::function& ) noexcept
  {
    keyword("end"sv);
    endl();
    endl();
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

  void ir_printer::endl() noexcept
  {
    out() << '\n';
  }

  void ir_printer::keyword(string_t kw) noexcept
  {
    fmt::print(out(), fmt::clr::BoldBlue, kw);
  }

  void ir_printer::name(string_t n) noexcept
  {
    fmt::print(out(), fmt::clr::BoldCyan, n);
  }

  void ir_printer::id(entity_id i) noexcept
  {
    fmt::print(out(), fmt::clr::Yellow, i);
  }

  void ir_printer::param(param_idx pi) noexcept
  {
    fmt::add_clr(out(), fmt::clr::BoldWhite);
    out() << "%`" << pi;
    fmt::clear_clr(out());
  }

}