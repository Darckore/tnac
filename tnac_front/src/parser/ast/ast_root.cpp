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


  // Root

  root::~root() noexcept = default;

  root::root() noexcept :
    node{ kind::Root }
  {}

}