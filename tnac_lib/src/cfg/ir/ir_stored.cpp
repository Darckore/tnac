#include "cfg/ir/ir_stored.hpp"

namespace tnac::ir // constant
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


namespace tnac::ir // record
{
  // Special members

  record::~record() noexcept = default;

  record::record(size_type size) noexcept :
    node{ kind::Record }
  {
    m_elems.reserve(size);
  }


  // Public members

  record& record::append(vreg& elem) noexcept
  {
    m_elems.push_back(&elem);
    return *this;
  }

  const vreg* record::get_element(size_type idx) const noexcept
  {
    return idx < m_elems.size() ? m_elems[idx] : nullptr;
  }
  vreg* record::get_element(size_type idx) noexcept
  {
    return FROM_CONST(get_element, idx);
  }
}