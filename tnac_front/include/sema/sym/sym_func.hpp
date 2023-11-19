//
// Functions
//

#pragma once
#include "sema/sym/sym_base.hpp"

namespace tnac::semantics
{
  class parameter;

  //
  // Symbol corresponding to a function
  //
  class function : public symbol
  {
  public:
    using param_list = std::vector<parameter*>;
    using size_type  = param_list::size_type;

  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(function);

    virtual ~function() noexcept;

  protected:
    function(scope& owner, name_t name, param_list params, scope& owned, loc_t loc) noexcept;

    function(kind k, scope& owner, name_t name, param_list params, scope& owned, loc_t loc) noexcept;

  public:
    //
    // Returns the numbers of parameters
    // Same as params().size(), just shorter
    //
    size_type param_count() const noexcept;

    //
    // Returns the list of function parameters
    // 
    // const version
    //
    const param_list& params() const noexcept;

    //
    // Returns the list of function parameters
    //
    param_list& params() noexcept;

    //
    // Returns the underlying scope
    // 
    // const version
    //
    const scope& owned_scope() const noexcept;

    //
    // Returns the underlying scope
    //
    scope& owned_scope() noexcept;

  protected:
    //
    // Allows derived classes to attach parameter lists later
    //
    void attach_params(param_list params) noexcept;

  private:
    param_list m_params;
    scope* m_ownedScope{};
  };
}