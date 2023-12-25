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
    auto&& mod = m_builder->make_module(id, name, conv_param_count(paramCount));
    m_modules.push_back(&mod);
    return mod;
  }

  function& cfg::declare_function(entity_id id, function& owner, name_t name, size_type paramCount) noexcept
  {
    return m_builder->make_function(id, owner, name, conv_param_count(paramCount));
  }

  function* cfg::find_entity(entity_id id) noexcept
  {
    return m_builder->find_function(id);
  }


  // Private members

  function::size_type cfg::conv_param_count(size_type paramCount) noexcept
  {
    UTILS_ASSERT(paramCount <= std::numeric_limits<function::size_type>::max());
    return static_cast<function::size_type>(paramCount);
  }
}