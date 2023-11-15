//
// Root nodes
//

#pragma once
#include "parser/ast/ast_base.hpp"

namespace tnac::semantics
{
  class module_ref;
}

namespace tnac::ast
{
  //
  // Module definition
  //
  class module_def final : public scope
  {
  public:
    using name_t = string_t;
    using loc_t  = src::loc_wrapper;
    using sym_t  = semantics::module_ref;

  private:
    friend class builder;
    friend class sema;

  public:
    CLASS_SPECIALS_NONE(module_def);

    virtual ~module_def() noexcept;

  protected:
    module_def(buf_t name, loc_t loc) noexcept;

    //
    // Attaches a symbol to this module. Called from sema on symbol creation
    //
    void attach_symbol(sym_t& sym) noexcept;

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
    // const version
    //
    const sym_t& symbol() const noexcept;

    //
    // Returns the attached symbol
    // 
    sym_t& symbol() noexcept;

  protected:
    //
    // Overrides the location with a new one
    //
    void override_loc(loc_t loc) noexcept;

  private:
    buf_t m_name;
    loc_t m_loc;
    sym_t* m_sym{};
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
    // const version
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