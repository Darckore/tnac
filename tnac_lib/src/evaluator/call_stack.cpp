#include "evaluator/call_stack.hpp"

namespace tnac::eval
{
  namespace detail
  {
    class stack_frame final
    {
    public:
      using func_name  = call_stack::func_name;
      using value_list = call_stack::value_list;

    public:
      CLASS_SPECIALS_NODEFAULT(stack_frame);

      stack_frame(func_name name, value_list args) noexcept :
        m_name{ name },
        m_args{ std::move(args) }
      {}

    private:
      func_name m_name;
      value_list m_args;
    };
  }

  // Special members

  call_stack::~call_stack() noexcept = default;

  call_stack::call_stack(size_type depth) noexcept :
    m_depth{ depth }
  {
    m_frames.reserve(depth);
  }

  call_stack::operator bool() const noexcept
  {
    return can_push();
  }


  // Public members

  bool call_stack::push(func_name funcName, value_list&& funcArgs) noexcept
  {
    if (!can_push())
      return false;

    m_frames.emplace_back(funcName, std::move(funcArgs));
    return true;
  }

  void call_stack::pop() noexcept
  {
    if (!m_frames.empty())
      m_frames.pop_back();
  }


  // Private members

  bool call_stack::can_push() const noexcept
  {
    return m_frames.size() < m_depth;
  }

}