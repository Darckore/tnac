#include "eval/stack/stack_frame.hpp"

namespace tnac::eval
{
  // Special members

  stack_frame::~stack_frame() noexcept = default;

  stack_frame::stack_frame(name_type fname, param_count argSz, entity_id jmpBack) noexcept :
    m_name{ fname },
    m_jmp{ jmpBack }
  {
    m_mem.reserve(argSz);
  }


  // Public members

  stack_frame::name_type stack_frame::name() const noexcept
  {
    return m_name;
  }

  stack_frame& stack_frame::add_arg(value argVal) noexcept
  {
    m_mem.emplace_back(std::move(argVal));
    return *this;
  }

  stack_frame& stack_frame::store(entity_id id, value val) noexcept
  {
    UTILS_ASSERT(*id < m_mem.size());
    m_mem[*id] = std::move(val);
    return *this;
  }

  entity_id stack_frame::allocate() noexcept
  {
    const auto idx = m_mem.size();
    m_mem.emplace_back();
    return idx;
  }

  value stack_frame::value_for(entity_id id) const noexcept
  {
    const auto idx = *id;
    return (idx < m_mem.size()) ? m_mem[idx] : value{};
  }

  entity_id stack_frame::jump_back() const noexcept
  {
    return m_jmp;
  }
}