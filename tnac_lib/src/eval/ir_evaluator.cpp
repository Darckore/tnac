#include "eval/ir_evaluator.hpp"

namespace tnac::detail
{
  namespace
  {
    auto to_addr(entity_id id) noexcept
    {
      return reinterpret_cast<const ir::instruction*>(*id);
    }
  }
}

namespace tnac
{
  // Special members

  ir_eval::~ir_eval() noexcept = default;

  ir_eval::ir_eval(ir::cfg& cfg, eval::store& vals) noexcept :
    m_cfg{ &cfg },
    m_valStore{ &vals }
  { }


  // Public members

  eval::value ir_eval::result() const noexcept
  {
    return m_result;
  }

  void ir_eval::enter(const ir::function& func) noexcept
  {
    auto jmpBack = m_instrPtr ? m_instrPtr->next() : nullptr;
    m_curFrame = &m_stack.make_frame(func.name(), func.param_count(), jmpBack);
    m_instrPtr = &(*func.entry().begin());
  }

  void ir_eval::leave() noexcept
  {
    UTILS_ASSERT(m_curFrame);
    m_instrPtr = detail::to_addr(m_curFrame->jump_back());
    m_curFrame = m_stack.pop_frame();
  }

  void ir_eval::evaluate_current() noexcept
  {
    while (m_instrPtr)
    {
      m_instrPtr = m_instrPtr->next();
    }
  }
}