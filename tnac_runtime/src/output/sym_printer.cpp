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
      case Global:   out() << "Global";          break;
      case Module:   print_module(scope->mod()); break;
      case Function: print_func(scope->func());  break;
      case Block:    out() << "Internal";        break;
      }
      if (m_styles) fmt::clear_clr(out());

      if (kind != Global)
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

  void sym_printer::print_module(const semantics::module_sym& modRef) noexcept
  {
    if (m_styles) fmt::clear_clr(out());
    if (m_styles) fmt::add_clr(out(), fmt::clr::BoldCyan);
    out() << modRef.name();
    if (m_styles) fmt::clear_clr(out());
    print_params(modRef.params(), true);
  }

  void sym_printer::print_var(const semantics::variable& var) noexcept
  {
    if (m_styles) fmt::clear_clr(out());
    if (m_styles) fmt::add_clr(out(), fmt::clr::Cyan);
    out() << var.name();
    if (m_styles) fmt::clear_clr(out());
  }

  void sym_printer::print_param(const semantics::parameter& par) noexcept
  {
    if (m_styles) fmt::clear_clr(out());
    if (m_styles) fmt::add_clr(out(), fmt::clr::Yellow);
    out() << par.name();
    if (m_styles) fmt::clear_clr(out());
  }

  void sym_printer::print_func(const semantics::function& func) noexcept
  {
    if (m_styles) fmt::clear_clr(out());
    if (m_styles) fmt::add_clr(out(), fmt::clr::Cyan);
    out() << func.name();
    if (m_styles) fmt::clear_clr(out());
    print_params(func.params(), false);
  }

  void sym_printer::print_sym(const semantics::symbol& sym) noexcept
  {
    using enum semantics::sym_kind;
    switch (sym.what())
    {
    case Variable:  print_var(utils::cast<Variable>(sym));    break;
    case Function:  print_func(utils::cast<Function>(sym));   break;
    case Parameter: print_param(utils::cast<Parameter>(sym)); break;
    case Module:    print_module(utils::cast<Module>(sym));   break;

    default: break;
    }
  }

  void sym_printer::print_params(const params_t& params, bool omitIfEmpty) noexcept
  {
    if (omitIfEmpty && params.empty())
      return;

    out() << " (";

    auto paramCount = params.size();
    for (decltype(paramCount) idx{}; auto param : params)
    {
      print_sym(*param);
      ++idx;
      if (idx < paramCount)
        out() << ", ";
    }

    out() << ')';
  }
}