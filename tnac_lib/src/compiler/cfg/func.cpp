#include "compiler/cfg/func.hpp"

namespace tnac::comp
{
  // Special members

  func::~func() noexcept = default;

  func::func(name_t name, func* parent) noexcept :
    m_name{ std::move(name) },
    m_parent{ parent }
  {}


  // Public members

  string_t func::name() const noexcept
  {
    return m_name;
  }

  const func* func::parent() const noexcept
  {
    return m_parent;
  }

  func* func::parent() noexcept
  {
    return FROM_CONST(parent);
  }

}