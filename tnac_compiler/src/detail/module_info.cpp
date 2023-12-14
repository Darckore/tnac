#include "compiler/detail/module_info.hpp"

namespace tnac::detail
{
  // Special members

  module_info::~module_info() noexcept = default;

  module_info::module_info() noexcept = default;


  // Public members

  void module_info::store(module_sym& sym, module_def& def) noexcept
  {
    auto newIt = m_data.try_emplace(&sym, &def);
    UTILS_ASSERT(newIt.second || newIt.first->second == &def);
  }

  module_info::module_def* module_info::locate(module_sym& sym) noexcept
  {
    auto found = m_data.find(&sym);
    return found != m_data.end() ? found->second : nullptr;
  }
}