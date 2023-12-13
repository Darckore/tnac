//
// IR function
//

#pragma once
#include "cfg/ir/ir_base.hpp"
#include "sema/sym/symbols.hpp"

namespace tnac::ir
{
  //
  // Represents IR functions and modules
  //
  class function final
  {
  public:
    using sym_t  = semantics::function;
    using mod_t  = semantics::module_sym;
    using name_t = sym_t::name_t;

  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(function);

    ~function() noexcept;

  protected:
    function(sym_t& sym, function* owner) noexcept;

  public:
    //
    // Returns the function or module name
    //
    name_t name() const noexcept;

    //
    // Returns a pointer to the owner function
    // 
    // const version
    //
    const function* owner_func() const noexcept;

    //
    // Returns a pointer to the owner function
    //
    function* owner_func() noexcept;

    //
    // Returns a reference to the underlying function symbol
    // 
    // const version
    //
    const sym_t& function_sym() const noexcept;

    //
    // Returns a reference to the underlying function symbol
    //
    sym_t& function_sym() noexcept;

    //
    // Attempts to cast the underlying symbol to a module and returns a pointer to it
    // nullptr if the cast is unsuccessful
    // 
    // const version
    //
    const mod_t* to_module() const noexcept;

    //
    // Attempts to cast the underlying symbol to a module and returns a pointer to it
    // nullptr if the cast is unsuccessful
    //
    mod_t* to_module() noexcept;

  private:
    function* m_owner{};
    sym_t* m_symbol{};
  };
}