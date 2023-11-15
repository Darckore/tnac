#include "sema/sym/sym_base.hpp"

namespace tnac::semantics
{
  // Special members

  symbol::~symbol() noexcept = default;

  symbol::symbol(kind k, name_t name, scope& owner, loc_t loc) noexcept :
    m_owner{ &owner },
    m_name{ name },
    m_location{ loc },
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
  scope& symbol::owner_scope() noexcept
  {
    return FROM_CONST(owner_scope);
  }

  string_t symbol::name() const noexcept
  {
    return m_name;
  }

  symbol::loc_t symbol::at() const noexcept
  {
    return m_location;
  }


  // Protected members

  void symbol::override_location(loc_t loc) noexcept
  {
    m_location = loc;
  }

}