#include "output/sym_printer.hpp"

namespace tnac::rt::out
{
  // Special members

  sym_printer::~sym_printer() noexcept = default;

  sym_printer::sym_printer() noexcept = default;

  // Public members

  void sym_printer::enable_styles() noexcept
  {
    m_styles = true;
  }


  // Private members

  out_stream& sym_printer::out() noexcept
  {
    return *m_out;
  }

  void sym_printer::print_scope(const semantics::scope* scope) noexcept
  {
    using enum semantics::scope_kind;
    if (scope)
    {
      if (m_styles) fmt::add_clr(out(), fmt::clr::White);
      const auto kind = scope->kind();
      switch (kind)
      {
      case Global:   out() << "Global";         break;
      case Module:   out() << "Module";         break;
      case Function: print_func(scope->func()); break;
      case Block:    out() << "Internal";       break;
      }
      if (m_styles) fmt::clear_clr(out());

      if (utils::eq_any(kind, Function, Block))
      {
        out() << "<=";
        print_scope(scope->encl_skip_internal());
      }
    }
    else
    {
      if (m_styles) fmt::add_clr(out(), fmt::clr::Red);
      out() << "UNKNOWN";
      if (m_styles) fmt::clear_clr(out());
    }
  }

  void sym_printer::print_var(const semantics::variable& var) noexcept
  {
    if (m_styles) fmt::add_clr(out(), fmt::clr::Cyan);
    out() << var.name();
    if (m_styles) fmt::clear_clr(out());
  }

  void sym_printer::print_param(const semantics::parameter& par) noexcept
  {
    if (m_styles) fmt::add_clr(out(), fmt::clr::Yellow);
    out() << par.name();
    if (m_styles) fmt::clear_clr(out());
  }

  void sym_printer::print_func(const semantics::function& func) noexcept
  {
    if (m_styles) fmt::add_clr(out(), fmt::clr::Cyan);
    out() << func.name();
    if (m_styles) fmt::clear_clr(out());

    out() << " (";

    auto paramCount = func.param_count();
    for (decltype(paramCount) idx{}; auto param : func.params())
    {
      print_sym(*param);
      ++idx;
      if (idx < paramCount)
        out() << ", ";
    }

    out() << ')';
  }

  void sym_printer::print_sym(const semantics::symbol& sym) noexcept
  {
    using enum semantics::sym_kind;
    switch (sym.what())
    {
    case Variable:  print_var(utils::cast<Variable>(sym));    break;
    case Function:  print_func(utils::cast<Function>(sym));   break;
    case Parameter: print_param(utils::cast<Parameter>(sym)); break;

    default: break;
    }
  }
}