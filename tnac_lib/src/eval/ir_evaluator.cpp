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

  void ir_eval::clear_env() noexcept
  {
    m_env.clear();
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
    while (step())
    {
    }
  }

  bool ir_eval::step() noexcept
  {
    if (!m_instrPtr)
      return false;

    dispatch();
    return true;
  }


  // Private members

  const ir::instruction& ir_eval::cur() const noexcept
  {
    return *m_instrPtr;
  }

  ir_eval::val_opt ir_eval::get_value(const ir::operand& op) const noexcept
  {
    val_opt res{};
    if (op.is_value())
    {
      res.emplace(op.get_value());
    }
    else if (op.is_register())
    {
      auto&& reg = op.get_reg();
      const auto regId = get_reg(reg);
      res.emplace(m_curFrame->value_for(regId));
    }

    return res;
  }

  entity_id ir_eval::get_reg(const ir::vreg& reg) const noexcept
  {
    const auto regId = m_env.find_reg(&reg);
    UTILS_ASSERT(regId);
    return regId.value_or(entity_id{});
  }

  void ir_eval::store_value(entity_id reg, const ir::operand& from) noexcept
  {
    auto fromVal = get_value(from);
    UTILS_ASSERT(fromVal);
    m_curFrame->store(reg, std::move(*fromVal));
  }

  entity_id ir_eval::alloc_new(const ir::operand& op) noexcept
  {
    UTILS_ASSERT(op.is_register());
    auto&& target = op.get_reg();
    const auto regId = m_curFrame->allocate();
    m_env.map(&target, regId);
    return regId;
  }

  void ir_eval::dispatch() noexcept
  {
    using enum ir::op_code;
    const auto opcode = m_instrPtr->opcode();

    SCOPE_GUARD(m_instrPtr = m_instrPtr->next());
    if (opcode == Alloc)
    {
      alloc();
      return;
    }
    if (opcode == Store)
    {
      store();
      return;
    }
    if (opcode == Load)
    {
      load();
      return;
    }
  }

  void ir_eval::alloc() noexcept
  {
    auto&& instr = cur();
    auto&& allocRes = instr[0];
    alloc_new(allocRes);
  }

  void ir_eval::store() noexcept
  {
    auto&& instr = cur();
    auto&& from = instr[0];
    auto&& to = instr[1];

    UTILS_ASSERT(to.is_register());
    const auto toReg = get_reg(to.get_reg());
    store_value(toReg, from);
  }

  void ir_eval::load() noexcept
  {
    auto&& instr = cur();
    auto&& to = instr[0];
    auto&& from = instr[1];

    if (!from.is_param())
    {
      const auto regId = alloc_new(to);
      store_value(regId, from);
      return;
    }

    UTILS_ASSERT(to.is_register());
    auto&& target = to.get_reg();
    auto par = from.get_param();
    m_env.map(&target, *par);
  }
}