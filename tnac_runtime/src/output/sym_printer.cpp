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
      const auto kind = scope->kind();
      switch (kind)
      {
      case Global:   fmt::print(out(), fmt::clr::White, "Global"sv); break;
      case Module:   print_module(scope->mod());                     break;
      case Function: print_func(scope->func());                      break;
      case Block:    print_internal_scope(*scope);                   break;
      }

      if (kind != Global)
      {
        out() << "<=";
        print_scope(scope->encl_skip_internal());
      }
    }
    else
    {
      fmt::print(out(), fmt::clr::Red, "UNKNOWN"sv);
    }
  }

  void sym_printer::print_module(const semantics::module_sym& modRef) noexcept
  {
    fmt::print(out(), fmt::clr::BoldCyan, modRef.name());
    print_params(modRef.params(), true);
  }

  void sym_printer::print_var(const semantics::variable& var) noexcept
  {
    fmt::print(out(), fmt::clr::Cyan, var.name());
  }

  void sym_printer::print_param(const semantics::parameter& par) noexcept
  {
    fmt::print(out(), fmt::clr::Yellow, par.name());
  }

  void sym_printer::print_func(const semantics::function& func) noexcept
  {
    fmt::print(out(), fmt::clr::Cyan, func.name());
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
    fmt::print(out(), fmt::clr::BoldWhite, sr.name());
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