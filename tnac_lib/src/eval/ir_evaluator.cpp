#include "eval/ir_evaluator.hpp"
#include "common/feedback.hpp"

namespace tnac::detail
{
  namespace
  {
    auto to_addr(entity_id id) noexcept
    {
      return reinterpret_cast<const ir::instruction*>(*id);
    }

    constexpr auto is_unary(ir::op_code oc) noexcept
    {
      using enum ir::op_code;
      return utils::eq_any(oc, Abs, CmpNot, CmpIs, Plus, Neg, BNeg, Head, Tail);
    }
    constexpr auto to_unary_op(ir::op_code oc) noexcept
    {
      using enum ir::op_code;
      using eval::val_ops;
      switch (oc)
      {
      case Abs:    return val_ops::AbsoluteValue;
      case CmpNot: return val_ops::LogicalNot;
      case CmpIs:  return val_ops::LogicalIs;
      case Plus:   return val_ops::UnaryPlus;
      case Neg:    return val_ops::UnaryNegation;
      case BNeg:   return val_ops::UnaryBitwiseNot;
      case Head:   return val_ops::UnaryHead;
      case Tail:   return val_ops::PostTail;
      }

      return val_ops::InvalidOp;
    }

    constexpr auto is_binary(ir::op_code oc) noexcept
    {
      using enum ir::op_code;
      return utils::eq_any(oc, Add, Sub, Mul, Div, Mod, Pow, Root, And, Or, Xor, 
                               CmpE, CmpL, CmpLE, CmpNE, CmpG, CmpGE);
    }
    constexpr auto to_binary_op(ir::op_code oc) noexcept
    {
      using enum ir::op_code;
      using eval::val_ops;
      switch (oc)
      {
      case Add:    return val_ops::Addition;
      case Sub:    return val_ops::Subtraction;
      case Mul:    return val_ops::Multiplication;
      case Div:    return val_ops::Division;
      case Mod:    return val_ops::Modulo;
      case Pow:    return val_ops::BinaryPow;
      case Root:   return val_ops::BinaryRoot;
      case And:    return val_ops::BitwiseAnd;
      case Or:     return val_ops::BitwiseOr;
      case Xor:    return val_ops::BitwiseXor;
      case CmpE:   return val_ops::Equal;
      case CmpL:   return val_ops::RelLess;
      case CmpLE:  return val_ops::RelLessEq;
      case CmpNE:  return val_ops::NEqual;
      case CmpG:   return val_ops::RelGr;
      case CmpGE:  return val_ops::RelGrEq;
      }

      return val_ops::InvalidOp;
    }
  }
}

namespace tnac
{
  // Special members

  ir_eval::~ir_eval() noexcept = default;

  ir_eval::ir_eval(ir::cfg& cfg, eval::store& vals, feedback* fb) noexcept :
    m_cfg{ &cfg },
    m_valStore{ &vals },
    m_feedback{ fb }
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
    auto&& entry = func.entry();
    m_branching.push({ nullptr, &entry });
    m_instrPtr = &(*entry.begin());
  }

  void ir_eval::leave() noexcept
  {
    if (!m_curFrame)
    {
      m_instrPtr = nullptr;
      return;
    }

    m_instrPtr = detail::to_addr(m_curFrame->jump_back());
    m_curFrame = m_stack.pop_frame();
    m_branching.pop();
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
    store_value(reg, std::move(*fromVal));
  }

  void ir_eval::store_value(entity_id reg, eval::value val) noexcept
  {
    m_curFrame->store(reg, val);
    m_result = std::move(val);
  }

  entity_id ir_eval::alloc_new(const ir::operand& op) noexcept
  {
    UTILS_ASSERT(op.is_register());
    auto&& target = op.get_reg();
    const auto regId = m_curFrame->allocate();
    m_env.map(&target, regId);
    return regId;
  }

  void ir_eval::jump_to(const ir::operand& op) noexcept
  {
    UTILS_ASSERT(op.is_block());
    auto&& block = op.get_block();
    jump_to(block);
  }

  void ir_eval::jump_to(const ir::basic_block& block) noexcept
  {
    UTILS_ASSERT(!m_branching.empty());
    auto&& br = m_branching.top();
    br.m_from = br.m_to;
    br.m_to = &block;

    m_instrPtr = &(*block.begin());
  }

  void ir_eval::dispatch() noexcept
  {
    using enum ir::op_code;
    const auto opcode = m_instrPtr->opcode();

    // Instructions which involve forced jumps go here:

    if (opcode == Jump)
    {
      jump();
      return;
    }
    if (opcode == Call)
    {
      call();
      return;
    }
    if (opcode == Ret)
    {
      ret();
      return;
    }

    // Non-jump instructions go here:

    SCOPE_GUARD(m_instrPtr = m_instrPtr->next());
    if (opcode == Alloc)
      alloc();
    else if (opcode == Store)
      store();
    else if (opcode == Load)
      load();
    else if (opcode == Test)
      test_type();
    else if (opcode == Phi)
      phi();
    else if (detail::is_unary(opcode))
      unary(opcode);
    else if (detail::is_binary(opcode))
      binary(opcode);

    /*
    Arr,
    Append,

    Select,

    DynBind,

    Bool,
    Int,
    Float,
    Frac,
    Cplx,
    */
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

  void ir_eval::jump() noexcept
  {
    auto&& instr = cur();
    if (instr.operand_count() == 1)
    {
      auto&& op = instr[0];
      jump_to(op);
      return;
    }

    auto&& cond = instr[0];
    auto&& ifTrue = instr[1];
    auto&& ifFalse = instr[2];

    auto condVal = get_value(cond);
    UTILS_ASSERT(condVal);

    if (eval::to_bool(*condVal))
      jump_to(ifTrue);
    else
      jump_to(ifFalse);
  }

  void ir_eval::phi() noexcept
  {
    auto&& instr = cur();
    auto&& to = instr[0];

    const auto regId = alloc_new(to);

    UTILS_ASSERT(!m_branching.empty());
    auto&& br = m_branching.top();

    for (auto idx = op_count{ 1 }; idx < instr.operand_count(); ++idx)
    {
      auto&& op = instr[idx];
      UTILS_ASSERT(op.is_edge());
      auto&& edge = op.get_edge();
      if (&edge.incoming() != br.m_from)
        continue;

      store_value(regId, edge.value());
      return;
    }
  }

  void ir_eval::unary(ir::op_code oc) noexcept
  {
    auto&& instr = cur();
    auto&& res = instr[0];
    auto&& operand = instr[1];

    const auto regId = alloc_new(res);
    const auto opId = detail::to_unary_op(oc);

    auto opVal = get_value(operand);
    UTILS_ASSERT(opVal);
    store_value(regId, opVal->unary(opId));
  }

  void ir_eval::binary(ir::op_code oc) noexcept
  {
    auto&& instr = cur();
    auto&& res = instr[0];
    auto&& lhs = instr[1];
    auto&& rhs = instr[2];

    const auto regId = alloc_new(res);
    const auto opId = detail::to_binary_op(oc);

    auto lv = get_value(lhs);
    auto rv = get_value(rhs);
    UTILS_ASSERT(lv);
    UTILS_ASSERT(rv);
    store_value(regId, lv->binary(opId, *rv));
  }

  void ir_eval::test_type() noexcept
  {
    auto&& instr = cur();
    auto&& res = instr[0];
    auto&& type = instr[1];
    auto&& val = instr[2];

    UTILS_ASSERT(type.is_typeid());
    const auto regId = alloc_new(res);

    const auto typeId = type.get_typeid();
    auto opVal = get_value(val);
    
    UTILS_ASSERT(opVal);
    store_value(regId, eval::value{ opVal->id() == typeId });
  }

  void ir_eval::call() noexcept
  {
    auto&& instr = cur();
    auto&& to = instr[0];
    auto&& f = instr[1];
    const auto argCount = instr.operand_count() - 2;

    const auto regId = alloc_new(to);
    auto callable = get_value(f).value_or(eval::value{}).try_get<eval::function_type>();
    if (!callable)
    {
      // todo: error & abort
      return;
    }

    auto&& func = *callable;
    if(func->param_count() != argCount)
    {
      // todo: error & abort
      return;
    }

    auto prevFrame = m_curFrame;
    enter(*func);
    m_curFrame->attach_ret_val(regId);
    auto nextFrame = m_curFrame;
    m_curFrame = prevFrame;
    for (auto idx = op_count{ 2 }; idx < instr.operand_count(); ++idx)
    {
      auto arg = get_value(instr[idx]);
      UTILS_ASSERT(arg);
      nextFrame->add_arg(std::move(*arg));
    }
    m_curFrame = nextFrame;
  }

  void ir_eval::ret()
  {
    auto&& instr = cur();
    auto&& op = instr[0];
    const auto retAddr = m_curFrame->ret_val();

    auto retVal = get_value(op);
    UTILS_ASSERT(retVal);

    auto cur = m_curFrame;
    auto retFrame = m_curFrame->prev();

    // Root
    if (!retFrame)
    {
      m_result = std::move(*retVal);
      leave();
      return;
    }

    m_curFrame = retFrame;
    store_value(retAddr, std::move(*retVal));
    m_curFrame = cur;
    leave();
  }
}