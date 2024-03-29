//
// Symbol printer
//

#pragma once
#include "output/common.hpp"
#include "output/formatting.hpp"
#include "sema/sym/sym_table.hpp"

namespace tnac::rt::out
{
  //
  // Printer for symbols
  // Outputs info about declared entities grouped by scope
  //
  class sym_printer final
  {
  public:
    using params_t = std::vector<semantics::parameter*>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(sym_printer);

    ~sym_printer() noexcept;

    sym_printer() noexcept;

  public:
    template <semantics::sem_symbol S>
    void operator()(semantics::sym_container<S> collection, out_stream& os) noexcept
    {
      m_out = &os;
      for (auto it = collection.begin(); it != collection.end(); ++it)
      {
        out() << "In scope '";
        print_scope(it.scope());
        out() << "':\n";
        for (auto sym : *it)
        {
          out() << ' ';
          print_sym(*sym);
          out() << " at ";
          fmt::println(out(), fmt::clr::DarkGray, sym->at());
        }

        if (std::next(it) != collection.end())
          out() << '\n';
      }
    }

    template <semantics::sem_symbol S>
    void operator()(semantics::sym_container<S> collection) noexcept
    {
      operator()(collection, out());
    }

  private:
    out_stream& out() noexcept;

    void print_scope(const semantics::scope* scope) noexcept;

    void print_module(const semantics::module_sym& modRef) noexcept;

    void print_var(const semantics::variable& var) noexcept;

    void print_param(const semantics::parameter& par) noexcept;

    void print_func(const semantics::function& func) noexcept;

    void print_internal_scope(const semantics::scope& scope) noexcept;

    void print_scope_ref(const semantics::scope_ref& sr) noexcept;

    void print_sym(const semantics::symbol& sym) noexcept;

    void print_params(const params_t& params, bool omitIfEmpty) noexcept;

  private:
    out_stream* m_out{ &std::cout };
  };
}