//
// Functions
//

#pragma once
#include "sema/sym/sym_base.hpp"

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
  class module_sym final : public symbol
  {
  public:
    using param_list = std::vector<parameter*>;
    using size_type  = param_list::size_type;

  private:
    friend class sym_table;
    friend class sema;

  public:
    CLASS_SPECIALS_NONE(module_sym);

    virtual ~module_sym() noexcept;

  protected:
    module_sym(scope& owner, name_t name, loc_t loc) noexcept;

  public:
    size_type param_count() const noexcept;

    const param_list& params() const noexcept;
    param_list& params() noexcept;

  protected:
    void attach_params(param_list params) noexcept;

    void override_location(loc_t loc) noexcept;

  private:
    param_list m_params;
  };
}