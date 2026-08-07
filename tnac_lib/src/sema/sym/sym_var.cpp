#include "sema/sym/sym_var.hpp"

namespace tnac::semantics // Variable
{
  // Special members

  variable::~variable() noexcept = default;

  variable::variable(scope& owner, name_t name, loc_t loc) noexcept :
    symbol{ kind::Variable, name, owner, loc }
  {}

  // Protected members

  void variable::mark_capture() noexcept
  {
    m_isCapture = true;
  }

  // Public members

  bool variable::is_capture() const noexcept
  {
    return m_isCapture;
  }

}


namespace tnac::semantics // Parameter
{
  // Special members

  parameter::~parameter() noexcept = default;

  parameter::parameter(scope& owner, name_t name, loc_t loc) noexcept :
    symbol{ kind::Parameter, name, owner, loc }
  {}
}