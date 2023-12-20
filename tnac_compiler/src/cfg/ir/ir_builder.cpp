#include "cfg/ir/ir_builder.hpp"

namespace tnac::ir
{
  // Special members

  builder::~builder() noexcept = default;

  builder::builder() noexcept = default;


  // Public members

  function& builder::make_module(entity_id id, fname_t name, par_size_t paramCount) noexcept
  {
    return make_function(id, {}, name, paramCount);
  }

  function& builder::make_function(entity_id id, function& owner, fname_t name, par_size_t paramCount) noexcept
  {
    return make_function(id, &owner, name, paramCount);
  }

  function* builder::find_function(entity_id id) noexcept
  {
    auto fIt = m_functions.find(id);
    return fIt != m_functions.end() ? &fIt->second : nullptr;
  }


  // Private members

  function& builder::make_function(entity_id id, function* owner, fname_t name, par_size_t paramCount) noexcept
  {
    auto blockIt = m_blocks.try_emplace(id);
    UTILS_ASSERT(blockIt.second);
    auto blocks = detail::block_container{ blockIt.first->second };

    auto newIt = owner ?
      m_functions.try_emplace(id, name, paramCount, *owner, std::move(blocks)) :
      m_functions.try_emplace(id, name, paramCount, std::move(blocks));
    UTILS_ASSERT(newIt.second);

    return newIt.first->second;
  }
}