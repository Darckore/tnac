//
// Sema
//

#pragma once
#include "sema/sym_table.hpp"

namespace tnac
{
  namespace detail
  {
    template <typename F>
    concept decl_handler = std::is_nothrow_invocable_r_v<void, F, const semantics::symbol&>;
  }

  //
  // Semantic analyser. Constrols scope tracking and is responsible
  // for registering and looking up symbols
  //
  class sema
  {
  public:
    using symbol = semantics::symbol;
    using sym_ptr = symbol*;
    using decl_handler_t = std::function<void(const symbol&)>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(sema);

    ~sema() noexcept;
    sema() noexcept;

  public:
    //
    // Sets a callback invoked when a variable is declared
    //
    template <detail::decl_handler F>
    void on_variable(F&& f) noexcept
    {
      m_varCallback = std::forward<F>(f);
    }

    //
    // Opens a new scope
    //
    void open_scope(const ast::scope& scopeNode) noexcept;

    //
    // Closes the current scope and makes its parent the current one
    //
    void close_scope() noexcept;

    //
    // Checks whether the specified symbol has previously been defined
    // and returns it if it was
    //
    sym_ptr find(string_t name) noexcept;

    //
    // Visits a newly created declarator and registers it in the symbol table
    //
    void visit_decl(ast::decl& decl) noexcept;

  private:
    semantics::sym_table m_symTab;
    const semantics::scope* m_curScope{};
    decl_handler_t m_varCallback{};
  };
}