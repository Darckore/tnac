#include "cfg/cfg.hpp"

namespace tnac::ir
{
  // Special members

  cfg::~cfg() noexcept = default;

  cfg::cfg(builder& bld) noexcept :
    m_builder{ &bld }
  {}


  // Public members

  function& cfg::declare_module(entity_id id, name_t name, size_type paramCount) noexcept
  {
    UTILS_ASSERT(paramCount <= std::numeric_limits<function::size_type>::max());
    return m_builder->make_module(id, name, static_cast<function::size_type>(paramCount));
  }

  function* cfg::find_module(entity_id id) noexcept
  {
    return m_builder->find_function(id);
  }

}