#include "parser/ast/ast.hpp"

namespace tnac::ast
{
  // Module definition

  module_def::~module_def() noexcept = default;

  module_def::module_def(buf_t name, loc_t loc) noexcept :
    scope{ kind::Module },
    m_name{ std::move(name) },
    m_loc{ loc }
  {}

  void module_def::attach_symbol(sym_t& sym) noexcept
  {
    m_sym = &sym;
  }

  module_def::name_t module_def::name() const noexcept
  {
    return m_name;
  }

  module_def::loc_t module_def::at() const noexcept
  {
    return m_loc;
  }

  void module_def::override_loc(loc_t loc) noexcept
  {
    m_loc = loc;
  }

  bool module_def::is_fake() const noexcept
  {
    return m_name.empty() && !m_loc;
  }

  const module_def::sym_t& module_def::symbol() const noexcept
  {
    UTILS_ASSERT(static_cast<bool>(m_sym));
    return *m_sym;
  }
  module_def::sym_t& module_def::symbol() noexcept
  {
    return FROM_CONST(symbol);
  }


  // Root

  root::~root() noexcept = default;

  root::root() noexcept :
    node{ kind::Root }
  {}

  const root::elem_list& root::modules() const noexcept
  {
    return m_modules;
  }
  root::elem_list& root::modules() noexcept
  {
    return FROM_CONST(modules);
  }

  void root::append(reference modDef) noexcept
  {
    assume_ancestry(&modDef);
    m_modules.emplace_back(&modDef);
  }

}