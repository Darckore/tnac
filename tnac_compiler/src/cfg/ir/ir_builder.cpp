#include "cfg/ir/ir_builder.hpp"

namespace tnac::ir
{
  // Special members

  builder::~builder() noexcept = default;

  builder::builder() noexcept = default;


  // Public members

  function& builder::make_module(entity_id id, function::name_t name, function::size_type paramCount) noexcept
  {
    auto newIt = m_functions.try_emplace(id, name, paramCount);
    return newIt.first->second;
  }

  function* builder::find_function(entity_id id) noexcept
  {
    auto fIt = m_functions.find(id);
    return fIt != m_functions.end() ? &fIt->second : nullptr;
  }
}