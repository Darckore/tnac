#include "eval/stack/call_stack.hpp"

namespace tnac::eval
{
  // Special members

  call_stack::~call_stack() noexcept = default;

  call_stack::call_stack() noexcept = default;


  // Public members

  stack_frame& call_stack::make_frame(name_type name, param_count argSz, entity_id jmp) noexcept
  {
    return m_frames.emplace_back(name, argSz, jmp);
  }

  stack_frame* call_stack::pop_frame() noexcept
  {
    if (m_frames.empty())
      return {};

    auto res = m_frames.back().prev();
    m_frames.pop_back();
    return res;
  }
}