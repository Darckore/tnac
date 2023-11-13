#include "sema/sym/sym_var.hpp"

namespace tnac::semantics // Variable
{
  // Special members

  variable::~variable() noexcept = default;

  variable::variable(scope& owner, name_t name) noexcept :
    symbol{ kind::Variable, name, owner }
  {}
}


namespace tnac::semantics  // Parameter
{
  // Special members

  parameter::~parameter() noexcept = default;

  parameter::parameter(scope& owner, name_t name) noexcept :
    symbol{ kind::Parameter, name, owner }
  {}
}