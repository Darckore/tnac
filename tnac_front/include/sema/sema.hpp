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
  private:
    //
    // Holds a scope received from sema and restores it in the destructor
    //
    class scope_guard final
    {
    private:
      friend class sema;

    protected:
      scope_guard(sema& s, semantics::scope* newScope) noexcept;

    public:
      scope_guard() = delete;

      ~scope_guard() noexcept;

      scope_guard(const scope_guard& other) noexcept = delete;
      scope_guard& operator=(const scope_guard& other) noexcept = delete;

      scope_guard(scope_guard&& other) noexcept;
      scope_guard& operator=(scope_guard&& other) noexcept;

      explicit operator bool() const noexcept;

      semantics::scope* current() noexcept;
      semantics::scope* prev() noexcept;

    private:
      sema* m_sema;
      semantics::scope* m_scope{};
      bool m_alive{ true };
    };

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
    // Returns the current scope
    //
    semantics::scope* current_scope() noexcept;

    //
    // Assumes the given scope as the current one
    // and returns a guard to the previous scope
    //
    scope_guard assume_scope(semantics::scope& scope) noexcept;

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
    // Visits an alias declaration of an imported module
    //
    semantics::symbol& visit_import_alias(const token& id, semantics::module_sym& src) noexcept;

    //
    // Attempts to resolve scope based on an expression
    //
    scope_guard try_resolve_scope(ast::expr& expr) noexcept;

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
    // Attempts to extract a symbol from an expression
    //
    semantics::symbol* extract_sym(ast::expr& expr) const noexcept;

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