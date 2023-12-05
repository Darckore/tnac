#include "output/sym_printer.hpp"

namespace tnac::rt::out
{
  // Special members

  sym_printer::~sym_printer() noexcept = default;

  sym_printer::sym_printer() noexcept = default;


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
      fmt::add_clr(out(), fmt::clr::White);
      const auto kind = scope->kind();
      switch (kind)
      {
      case Global:   out() << "Global";            break;
      case Module:   print_module(scope->mod());   break;
      case Function: print_func(scope->func());    break;
      case Block:    print_internal_scope(*scope); break;
      }
      fmt::clear_clr(out());

      if (kind != Global)
      {
        out() << "<=";
        print_scope(scope->encl_skip_internal());
      }
    }
    else
    {
      fmt::add_clr(out(), fmt::clr::Red);
      out() << "UNKNOWN";
      fmt::clear_clr(out());
    }
  }

  void sym_printer::print_module(const semantics::module_sym& modRef) noexcept
  {
    fmt::clear_clr(out());
    fmt::add_clr(out(), fmt::clr::BoldCyan);
    out() << modRef.name();
    fmt::clear_clr(out());
    print_params(modRef.params(), true);
  }

  void sym_printer::print_var(const semantics::variable& var) noexcept
  {
    fmt::clear_clr(out());
    fmt::add_clr(out(), fmt::clr::Cyan);
    out() << var.name();
    fmt::clear_clr(out());
  }

  void sym_printer::print_param(const semantics::parameter& par) noexcept
  {
    fmt::clear_clr(out());
    fmt::add_clr(out(), fmt::clr::Yellow);
    out() << par.name();
    fmt::clear_clr(out());
  }

  void sym_printer::print_func(const semantics::function& func) noexcept
  {
    fmt::clear_clr(out());
    fmt::add_clr(out(), fmt::clr::Cyan);
    out() << func.name();
    fmt::clear_clr(out());
    print_params(func.params(), false);
  }

  void sym_printer::print_internal_scope(const semantics::scope& scope) noexcept
  {
    auto scRef = scope.to_scope_ref();
    if (!scRef)
    {
      out() << "Internal";
      return;
    }

    print_scope_ref(*scRef);
  }

  void sym_printer::print_scope_ref(const semantics::scope_ref& sr) noexcept
  {
    fmt::clear_clr(out());
    fmt::add_clr(out(), fmt::clr::BoldWhite);
    out() << sr.name();
    fmt::clear_clr(out());
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