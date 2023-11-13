#include "sema/sym/sym_base.hpp"
#include "parser/ast/ast_base.hpp"
#include "parser/ast/ast_decls.hpp"

namespace tnac::semantics
{
  // Base symbol

  symbol::~symbol() noexcept = default;

  symbol::symbol(kind k, name_t name, const scope& owner) noexcept :
    m_owner{ &owner },
    m_name{ name },
    m_kind{ k }
  {}

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

  // Variable symbol

  variable::~variable() noexcept = default;

  variable::variable(const scope& owner, name_t name) noexcept :
    symbol{ kind::Variable, name, owner }
  {}


  // Parameter symbol

  parameter::~parameter() noexcept = default;

  parameter::parameter(const scope& owner, name_t name) noexcept :
    symbol{ kind::Parameter, name, owner }
  {}


  // Function symbol

  function::~function() noexcept = default;

  function::function(const scope& owner, name_t name, param_list params) noexcept :
    symbol{ kind::Function, name, owner },
    m_params{ std::move(params) }
  {}

  function::size_type function::param_count() const noexcept
  {
    return m_params.size();
  }

  const function::param_list& function::params() const noexcept
  {
    return m_params;
  }
  function::param_list& function::params() noexcept
  {
    return FROM_CONST(params);
  }
}