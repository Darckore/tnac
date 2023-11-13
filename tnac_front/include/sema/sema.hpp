//
// Sema
//

#pragma once
#include "sema/sym/sym_table.hpp"
#include "parser/ast/ast.hpp"

namespace tnac
{
  //
  // Semantic analyser. Constrols scope tracking and is responsible
  // for registering and looking up symbols
  //
  class sema final
  {
  public:
    using symbol = semantics::symbol;
    using sym_ptr = symbol*;

    using fake_name_set = std::unordered_set<buf_t>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(sema);

    ~sema() noexcept;
    sema() noexcept;

  public:
    //
    // Opens a new scope
    //
    void open_scope() noexcept;

    //
    // Closes the current scope and makes its parent the current one
    //
    void close_scope() noexcept;

    //
    // Checks whether the specified symbol has previously been defined
    // and returns it if it was
    //
    sym_ptr find(string_t name, bool currentOnly = false) noexcept;

    //
    // Visits a newly created declarator and registers it in the symbol table
    //
    void visit_decl(ast::decl& decl) noexcept;

    //
    // Generates a random name
    //
    token contrive_name() noexcept;

    //
    // Returns an iterable collection of all declared variables
    //
    auto vars() const noexcept
    {
      return m_symTab.vars();
    }

    //
    // Returns an iterable collection of all declared functions
    //
    auto funcs() const noexcept
    {
      return m_symTab.funcs();
    }

  private:
    semantics::sym_table m_symTab;
    fake_name_set m_generatedNames;
    const semantics::scope* m_curScope{};
  };
}