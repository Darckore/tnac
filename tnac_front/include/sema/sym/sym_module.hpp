//
// Functions
//

#pragma once
#include "sema/sym/sym_base.hpp"
#include "sema/sym/sym_func.hpp"

namespace tnac
{
  class sema;
}

namespace tnac::semantics
{
  class parameter;

  //
  // Symbol corresponding to a module
  //
  class module_sym final : public function
  {
  private:
    friend class sym_table;
    friend class sema;

  public:
    CLASS_SPECIALS_NONE(module_sym);

    virtual ~module_sym() noexcept;

  protected:
    module_sym(scope& owner, name_t name, scope& modScope, loc_t loc) noexcept;

  protected:
    //
    // Allows sema to attach params
    // Needed since module entry definition is completely optional
    //
    void attach_params(param_list params) noexcept;

    //
    // Allows sema to attach location of the _entry definition
    // Needed since module entry definition is completely optional
    //
    void override_location(loc_t loc) noexcept;
  };


  //
  // Symbol which refers to a scope
  //
  class scope_ref final : public symbol
  {
  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(scope_ref);

    virtual ~scope_ref() noexcept;

  protected:
    scope_ref(scope& owner, name_t name, scope& referenced, loc_t loc) noexcept;

  public:
    //
    // Returns the scope it references
    //
    const scope& referenced() const noexcept;

    //
    // Returns the scope it references
    //
    scope& referenced() noexcept;

  private:
    scope* m_referenced{};
  };


  //
  // Symbol which referes to an unresolved, but potentially valid entity
  // Since typing is dynamic, the scope in which we need to loop a name up,
  // may be unknown during parsing, e.g:
  // func(x)
  //  internal()
  //    moreInternal() ;
  //  ;
  //  { x > 0 } -> { 42, internal } 
  // ;
  // func(-1).moreInternal()
  //         ^ needs evaluation to figure out
  //
  class deferred final : public symbol
  {
  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(deferred);

    virtual ~deferred() noexcept;

  protected:
    deferred(scope& owner, name_t name, loc_t loc) noexcept;
  };
}