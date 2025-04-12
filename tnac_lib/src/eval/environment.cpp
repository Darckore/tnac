#include "eval/environment.hpp"

namespace tnac::eval
{
  // Special members

  env::~env() noexcept = default;

  env::env() noexcept = default;


  // Public members

  void env::map(entity_id frame, entity_id ent, entity_id reg) noexcept
  {
    auto newFrame = m_map.try_emplace(frame, mem_frame{});
    auto&& fr = newFrame.first->second;
    fr.try_emplace(ent, reg);
  }

  env::reg_opt env::find_reg(entity_id frame, entity_id ent) const noexcept
  {
    auto frIt = m_map.find(frame);
    if (frIt == m_map.end())
      return {};

    auto&& fr = frIt->second;
    auto regIt = fr.find(ent);
    return regIt != fr.end() ?
      reg_opt{ regIt->second } :
      reg_opt{};
  }

  void env::remove_frame(entity_id frame) noexcept
  {
    m_map.erase(frame);
  }

  void env::clear() noexcept
  {
    m_map.clear();
  }
}