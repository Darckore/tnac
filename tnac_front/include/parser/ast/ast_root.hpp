//
// Root nodes
//

#pragma once
#include "parser/ast/ast_base.hpp"
#include "parser/ast/ast_decls.hpp"

namespace tnac::semantics
{
  class module_sym;
}

namespace tnac::ast
{
  class id_expr;
}

namespace tnac::ast
{
  //
  // Import directive
  //
  class import_dir final : public list<id_expr>, public node
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(import_dir);

    virtual ~import_dir() noexcept;

  protected:
    import_dir(const token& pos, elem_list name, pointer aliasName) noexcept;

  public:
    //
    // Returns the position of the 'import' keyword
    //
    const token& pos() const noexcept;

    //
    // Returns a reference to the name container
    //
    const elem_list& name() const noexcept;

    //
    // Returns a reference to the name container
    //
    elem_list& name() noexcept;

    //
    // Returns a reference to the imported symbol
    //
    const semantics::symbol& imported_sym() const noexcept;

    //
    // Returns a reference to the imported symbol
    //
    semantics::symbol& imported_sym() noexcept;

    //
    // Returns the alias for the imported module if
    // one is defined
    //
    const_pointer alias_name() const noexcept;

    //
    // Returns the alias for the imported module if
    // one is defined
    //
    pointer alias_name() noexcept;

  private:
    token m_pos;
    elem_list m_name;
    pointer m_aliasName{};
  };


  //
  // Module definition
  //
  class module_def final : public scope
  {
  public:
    using name_t = string_t;
    using loc_t  = src::loc_wrapper;
    using sym_t  = semantics::module_sym;

    using import_list = list<import_dir>::elem_list;
    using param_list  = list<param_decl>::elem_list;
    using size_type   = param_list::size_type;

  private:
    friend class builder;
    friend class sema;

  public:
    CLASS_SPECIALS_NONE(module_def);

    virtual ~module_def() noexcept;

  protected:
    module_def(buf_t name, loc_t loc) noexcept;

  public:
    //
    // Returns the module name
    //
    name_t name() const noexcept;

    //
    // Returns the location at which this module's header is defined
    //
    loc_t at() const noexcept;

    //
    // Checks whether the module is fake
    // This tells us whether the module is the default one
    // It will have an empty name and dummy location
    // 
    // Needed when parsing is called with just a string,
    // withot any source file at all
    //
    bool is_fake() const noexcept;

    //
    // Returns the attached symbol
    //
    const sym_t& symbol() const noexcept;

    //
    // Returns the attached symbol
    // 
    sym_t& symbol() noexcept;

    //
    // Returns module parameter list
    //
    const param_list& params() const noexcept;

    //
    // Returns module parameter list
    //
    param_list& params() noexcept;

    //
    // Returns the number of parameters
    //
    size_type param_count() const noexcept;

    //
    // Adds an import directive
    //
    void add_import(ast::import_dir& id) noexcept;

    //
    // Returns the import list
    //
    const import_list& imports() const noexcept;

    //
    // Returns the import list
    //
    import_list& imports() noexcept;

    //
    // Returns the number of imports
    //
    size_type import_count() const noexcept;

  protected:
    //
    // Overrides the location with a new one
    //
    void override_loc(loc_t loc) noexcept;

    //
    // Overrides the location with a new one
    //
    void attach_params(param_list params) noexcept;

    //
    // Attaches a symbol to this module. Called from sema on symbol creation
    //
    void attach_symbol(sym_t& sym) noexcept;

  private:
    buf_t m_name;
    loc_t m_loc;
    sym_t* m_sym{};
    param_list m_params;
    import_list m_imports;
  };


  //
  // Root node of the AST
  //
  class root final : public list<module_def>, public node
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(root);

    virtual ~root() noexcept;

  protected:
    root() noexcept;

  public:
    //
    // Returns the current list of modules
    //
    const elem_list& modules() const noexcept;

    //
    // Returns the current list of modules
    // 
    elem_list& modules() noexcept;

    //
    // Adds a new module
    //
    void append(reference modDef) noexcept;

  private:
    elem_list m_modules;
  };
}