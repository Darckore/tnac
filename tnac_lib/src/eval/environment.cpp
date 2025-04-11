#include "eval/environment.hpp"

namespace tnac::eval
{
  // Special members

  env::~env() noexcept = default;

  env::env() noexcept = default;


  // Public members

  void env::map(entity_id ent, entity_id reg) noexcept
  {
    m_map.try_emplace(ent, reg);
  }

  env::reg_opt env::find_reg(entity_id ent) const noexcept
  {
    auto found = m_map.find(ent);
    return found != m_map.end() ?
           reg_opt{ found->second } :
           reg_opt{};
  }

  void env::clear() noexcept
  {
    m_map.clear();
  }
}