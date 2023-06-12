#include "evaluator/call_stack.hpp"

namespace tnac::eval
{
  // Special members

  call_stack::~call_stack() noexcept = default;

  call_stack::call_stack(size_type depth) noexcept :
    m_depth{ depth }
  {}

  call_stack::operator bool() const noexcept
  {
    return !m_overflown;
  }
}