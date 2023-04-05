//
// Sema
//

#pragma once
#include "sema/sym_table.hpp"

namespace tnac
{
  //
  // Semantic analyser. Constrols scope tracking and is responsible
  // for registering and looking up symbols
  //
  class sema
  {
  public:
    using symbol = semantics::symbol;
    using sym_ptr = const symbol*;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(sema);

    ~sema() noexcept;
    sema() noexcept;

  public:
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
  };
}