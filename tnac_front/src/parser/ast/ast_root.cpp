#include "parser/ast/ast.hpp"

namespace tnac::ast // Import directive
{
  import_dir::~import_dir() noexcept = default;

  import_dir::import_dir(const token& pos, elem_list name) noexcept :
    node{ kind::Import },
    m_pos{ pos },
    m_name{ std::move(name) }
  {
    for (auto part : m_name)
      assume_ancestry(part);
  }

  const token& import_dir::pos() const noexcept
  {
    return m_pos;
  }

  const import_dir::elem_list& import_dir::name() const noexcept
  {
    return m_name;
  }
  import_dir::elem_list& import_dir::name() noexcept
  {
    return FROM_CONST(name);
  }
}


namespace tnac::ast // Module definition
{
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

  void module_def::attach_params(param_list params) noexcept
  {
    UTILS_ASSERT(m_params.empty());
    m_params = std::move(params);
    for (auto p : m_params)
      assume_ancestry(p);
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

  const module_def::param_list& module_def::params() const noexcept
  {
    return m_params;
  }
  module_def::param_list& module_def::params() noexcept
  {
    return FROM_CONST(params);
  }

  module_def::size_type module_def::param_count() const noexcept
  {
    return params().size();
  }

  void module_def::add_import(ast::import_dir& id) noexcept
  {
    m_imports.emplace_back(&id);
    assume_ancestry(&id);
  }

  const module_def::import_list& module_def::imports() const noexcept
  {
    return m_imports;
  }
  module_def::import_list& module_def::imports() noexcept
  {
    return FROM_CONST(imports);
  }

  module_def::size_type module_def::import_count() const noexcept
  {
    return m_imports.size();
  }
}


namespace tnac::ast // Root
{
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