#include "sema/sym/sym_base.hpp"

namespace tnac::semantics
{
  // Special members

  symbol::~symbol() noexcept = default;

  symbol::symbol(kind k, name_t name, const scope& owner) noexcept :
    m_owner{ &owner },
    m_name{ name },
    m_kind{ k }
  {}


  // Public members

  sym_kind symbol::what() const noexcept
  {
    return m_kind;
  }

  bool symbol::is(kind k) const noexcept
  {
    return what() == k;
  }

  const scope& symbol::owner_scope() const noexcept
  {
    return *m_owner;
  }

  string_t symbol::name() const noexcept
  {
    return m_name;
  }
}