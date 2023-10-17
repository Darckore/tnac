#include "compiler/cfg/environment.hpp"

namespace tnac::comp
{
  // Special members

  environment::~environment() noexcept = default;

  environment::environment() noexcept = default;


  // Public members

  [[nodiscard]]
  environment::reg_index_t environment::next_register() noexcept
  {
    m_regStack.push(m_curRegisterIdx);
    return m_curRegisterIdx++;
  }

  [[nodiscard]]
  environment::reg_index_t environment::pop_register() noexcept
  {
    UTILS_ASSERT(!m_regStack.empty());
    const auto res = m_regStack.top();
    m_regStack.pop();
    return res;
  }

}