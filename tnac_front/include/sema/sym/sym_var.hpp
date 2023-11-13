//
// Variables
//

#pragma once
#include "sema/sym/sym_base.hpp"

namespace tnac::semantics
{
  //
  // Symbol corresponding to a variable
  //
  class variable final : public symbol
  {
  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(variable);

    virtual ~variable() noexcept;

  protected:
    variable(scope& owner, name_t name) noexcept;
  };


  //
  // Symbol corresponding to a parameter
  //
  class parameter final : public symbol
  {
  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(parameter);

    virtual ~parameter() noexcept;

  protected:
    parameter(scope& owner, name_t name) noexcept;
  };
}