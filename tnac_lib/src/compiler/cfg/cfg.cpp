#include "compiler/cfg/cfg.hpp"

namespace tnac::comp
{
  // Special members

  cfg::~cfg() noexcept = default;

  cfg::cfg() noexcept = default;

  cfg::operator bool() const noexcept
  {
    return static_cast<bool>(m_entry);
  }


  // Public members

  basic_block& cfg::create(block_name name) noexcept
  {
    auto key = storage_key{ name };
    auto item = m_blocks.try_emplace(key, std::move(name));
    
    auto&& res = item.first->second;
    if (!m_entry)
      m_entry = &res;

    return res;
  }

  basic_block& cfg::entry() noexcept
  {
    UTILS_ASSERT(static_cast<bool>(*this));
    return *m_entry;
  }

  basic_block* cfg::find(storage_key name) noexcept
  {
    auto block = m_blocks.find(name);
    return block != m_blocks.end() ? &block->second : nullptr;
  }

}