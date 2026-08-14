#include "eval/stack/stack_frame.hpp"
#include "cfg/ir/ir_function.hpp"

namespace tnac::eval
{
  // Special members

  stack_frame::~stack_frame() noexcept = default;

  stack_frame::stack_frame(eval::function_type func, param_count argSz, entity_id jmpBack) noexcept :
    m_func{ std::move(func) },
    m_jmp{ jmpBack }
  {
    m_mem.reserve(argSz);
  }


  // Public members

  stack_frame::name_type stack_frame::name() const noexcept
  {
    return m_func->name();
  }

  eval::function_type stack_frame::function() const noexcept
  {
    return m_func;
  }

  entity_id stack_frame::add_arg(value argVal) noexcept
  {
    auto res = entity_id{ m_mem.size() };
    m_mem.emplace_back(std::move(argVal));
    return res;
  }

  void stack_frame::store(entity_id id, value val) noexcept
  {
    UTILS_ASSERT(*id < m_mem.size());
    m_mem[*id] = std::move(val);
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

  value stack_frame::value_for_this() const noexcept
  {
    return m_this != entity_id{} ? m_mem[*m_this] : value{};
  }

  void stack_frame::redirrect(entity_id jmp) noexcept
  {
    m_jmp = jmp;
  }

  entity_id stack_frame::jump_back() const noexcept
  {
    return m_jmp;
  }

  void stack_frame::attach_ret_val(entity_id rv) noexcept
  {
    m_retId = rv;
  }

  entity_id stack_frame::ret_val() const noexcept
  {
    return m_retId;
  }

  void stack_frame::init_this_reg(value val) noexcept
  {
    m_this = add_arg(std::move(val));
  }
}