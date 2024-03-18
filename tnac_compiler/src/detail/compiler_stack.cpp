#include "compiler/detail/compiler_stack.hpp"

namespace tnac::detail
{
  // Special members

  compiler_stack::~compiler_stack() noexcept = default;

  compiler_stack::compiler_stack() noexcept = default;


  // Public members

  void compiler_stack::push(ir::operand op) noexcept
  {
    m_data.emplace_back(op);
  }

  compiler_stack::value_type compiler_stack::top() noexcept
  {
    UTILS_ASSERT(!m_data.empty());
    return m_data.back();
  }

  void compiler_stack::pop() noexcept
  {
    if (m_data.empty()) return;
    m_data.pop_back();
  }

  compiler_stack::value_type compiler_stack::extract() noexcept
  {
    SCOPE_GUARD(pop());
    return top();
  }

  bool compiler_stack::empty() const noexcept
  {
    return m_data.empty();
  }

  // Private members


}