#include "compiler/detail/compiler_stack.hpp"
#include "eval/value/value.hpp"

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

  void compiler_stack::push_undef() noexcept
  {
    push(eval::stored_value{});
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

  void compiler_stack::drop(size_type count) noexcept
  {
    if (m_data.size() < count)
    {
      m_data.clear();
      return;
    }

    while (count--)
      pop();
  }

  compiler_stack::value_type compiler_stack::extract() noexcept
  {
    SCOPE_GUARD(pop());
    return top();
  }

  compiler_stack::value_type compiler_stack::try_extract() noexcept
  {
    if (empty())
      return eval::stored_value{};
    return extract();
  }

  bool compiler_stack::empty() const noexcept
  {
    return m_data.empty();
  }

  bool compiler_stack::has_values(size_type count) const noexcept
  {
    if (m_data.size() < count)
      return false;

    for (auto cnt = size_type{}; auto&& op : m_data | views::reverse)
    {
      if (cnt == count)
        break;

      if (!op.is_value())
        return false;

      ++cnt;
    }
    return true;
  }

  bool compiler_stack::has_at_least(size_type count) const noexcept
  {
    return m_data.size() >= count;
  }

  void compiler_stack::fill(ir::instruction& instr, size_type count) noexcept
  {
    walk_back(count, [&instr](auto op) noexcept
      {
        instr.add(std::move(op));
      });
  }

  void compiler_stack::fill(arr_type& arr, size_type count) noexcept
  {
    walk_back(count, [&arr](auto op) noexcept
      {
        UTILS_ASSERT(op.is_value());
        arr.emplace_back(op.get_value());
      });
  }

  // Private members

  void compiler_stack::walk_back(size_type count, op_processor auto&& proc) noexcept
  {
    if (!count)
      return;

    UTILS_ASSERT(m_data.size() >= count);
    auto beg = std::next(m_data.begin(), m_data.size() - count);
    for (auto it = beg; it < m_data.end(); ++it)
      proc(*it);

    while (count--)
      pop();
  }

}