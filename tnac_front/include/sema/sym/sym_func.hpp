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
  class function final : public symbol
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
    function(scope& owner, name_t name, param_list params, loc_t loc) noexcept;

  public:
    size_type param_count() const noexcept;

    const param_list& params() const noexcept;
    param_list& params() noexcept;

  private:
    param_list m_params;
  };
}