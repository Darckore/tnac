#include "cfg/ir/ir_function.hpp"
#include "sema/sym/symbols.hpp"

namespace tnac::ir
{
  // Special members

  function::~function() noexcept = default;

  function::function(sym_t& sym, function* owner) noexcept :
    m_owner{ owner },
    m_symbol{ &sym }
  {}


  // Public members

  const function* function::owner_func() const noexcept
  {
    return m_owner;
  }
  function* function::owner_func() noexcept
  {
    return FROM_CONST(owner_func);
  }

  const function::sym_t& function::function_sym() const noexcept
  {
    return *m_symbol;
  }
  function::sym_t& function::function_sym() noexcept
  {
    return FROM_CONST(function_sym);
  }

  const function::mod_t* function::to_module() const noexcept
  {
    return utils::try_cast<mod_t>(m_symbol);
  }
  function::mod_t* function::to_module() noexcept
  {
    return FROM_CONST(to_module);
  }
}