#include "cfg/ir/ir_stored.hpp"

namespace tnac::ir
{
  // Special members

  constant::~constant() noexcept = default;

  constant::constant(vreg& reg, value_type val) noexcept :
    node{ kind::Constant },
    m_reg{ &reg },
    m_value{ std::move(val) }
  {}


  // Public members

  const vreg& constant::target_reg() const noexcept
  {
    return *m_reg;
  }
  vreg& constant::target_reg() noexcept
  {
    return FROM_CONST(target_reg);
  }

  const constant::value_type& constant::value() const noexcept
  {
    return m_value;
  }
}