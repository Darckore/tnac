//
// Functions
//

#pragma once
#include "sema/sym/sym_base.hpp"

namespace tnac::semantics
{
  class parameter;

  //
  // Symbol corresponding to a module
  //
  class module_ref final : public symbol
  {
  public:
    using param_list = std::vector<parameter*>;
    using size_type  = param_list::size_type;

  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(module_ref);

    virtual ~module_ref() noexcept;

  protected:
    module_ref(scope& owner, name_t name, loc_t loc) noexcept;

  public:
    size_type param_count() const noexcept;

    const param_list& params() const noexcept;
    param_list& params() noexcept;

  private:
    param_list m_params;
  };
}