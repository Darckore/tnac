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
    CLASS_SPECIALS_NONE_CUSTOM(sym_printer);

    ~sym_printer() noexcept;

    sym_printer() noexcept;

  public:
    template <semantics::detail::sym S>
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

          if (m_styles) fmt::add_clr(out(), fmt::clr::White);
          out() << sym->at();
          if (m_styles) fmt::clear_clr(out());

          out() << '\n';
        }

        if (std::next(it) != collection.end())
          out() << '\n';
      }
    }

    template <semantics::detail::sym S>
    void operator()(semantics::sym_container<S> collection) noexcept
    {
      operator()(collection, out());
    }

    void enable_styles() noexcept;

  private:
    out_stream& out() noexcept;

    void print_scope(const semantics::scope* scope) noexcept;

    void print_var(const semantics::variable& var) noexcept;

    void print_param(const semantics::parameter& par) noexcept;

    void print_func(const semantics::function& func) noexcept;

    void print_sym(const semantics::symbol& sym) noexcept;

  private:
    out_stream* m_out{ &std::cout };
    bool m_styles{};
  };
}