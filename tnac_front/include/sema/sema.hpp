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

    using ast_params    = std::vector<ast::param_decl*>;
    using symbol_params = std::vector<semantics::parameter*>;
    using loc_t         = src::loc_wrapper;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(sema);

    ~sema() noexcept;
    sema() noexcept;

  public:
    //
    // Opens a new scope
    //
    void open_scope(semantics::scope_kind kind) noexcept;

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
    // Visits a newly created module definition and registers it in the symbol table
    //
    void visit_module_def(ast::module_def& def) noexcept;

    //
    // Visits an existing module definition and applies entry params and location
    //
    void visit_module_entry(ast::module_def& def, ast_params params, loc_t at) noexcept;

    //
    // Visits a part of an import name
    //
    semantics::symbol& visit_import_component(const token& id) noexcept;

    //
    // Generates a random name
    //
    string_t contrive_name() noexcept;

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

    //
    // Returns an iterable collection of all declared modules
    //
    auto modules() const noexcept
    {
      return m_symTab.modules();
    }

  private:
    //
    // Creates symbol params from AST-related params
    //
    symbol_params make_params(const ast_params& src) const noexcept;

  private:
    semantics::sym_table m_symTab;
    fake_name_set m_generatedNames;
    semantics::scope* m_curScope{};
  };
}