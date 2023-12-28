#include "compiler/detail/name_repo.hpp"

namespace tnac::detail
{
  // Special members

  name_repo::~name_repo() noexcept = default;

  name_repo::name_repo() noexcept = default;


  // Public members

  name_repo::idx_cache name_repo::init_indicies() noexcept
  {
    return m_prefNames;
  }

  string_t name_repo::entry_block_name() noexcept
  {
    return "entry"sv;
  }

  string_t name_repo::ret_block_name() noexcept
  {
    return "ret"sv;
  }

  string_t name_repo::mangle_module_name(string_t original, std::size_t parCnt) noexcept
  {
    static constexpr auto fmtStr = "{}`{}"sv;
    const auto pc = static_cast<param_count>(parCnt);
    return m_plainNames.format(fmtStr, original, pc);
  }

  string_t name_repo::mangle_func_name(string_t original, const ir::function& owner, std::size_t parCnt) noexcept
  {
    static constexpr auto fmtStr = "{}`{}@{:X}"sv;
    const auto pc = static_cast<param_count>(parCnt);
    const auto ownerEnt = entity_id{ &owner };
    return m_plainNames.format(fmtStr, original, pc, *ownerEnt);
  }
}