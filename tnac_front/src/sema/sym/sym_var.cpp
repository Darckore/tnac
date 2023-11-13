#include "sema/sym/sym_var.hpp"

namespace tnac::semantics // Variable
{
  // Special members

  variable::~variable() noexcept = default;

  variable::variable(scope& owner, name_t name, loc_t loc) noexcept :
    symbol{ kind::Variable, name, owner, loc }
  {}
}


namespace tnac::semantics  // Parameter
{
  // Special members

  parameter::~parameter() noexcept = default;

  parameter::parameter(scope& owner, name_t name, loc_t loc) noexcept :
    symbol{ kind::Parameter, name, owner, loc }
  {}
}