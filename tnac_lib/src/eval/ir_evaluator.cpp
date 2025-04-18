#include "eval/ir_evaluator.hpp"
#include "common/feedback.hpp"
#include "eval/value/type_impl.hpp"
#include "eval/value/traits.hpp"

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

    constexpr auto is_type(ir::op_code oc) noexcept
    {
      using enum ir::op_code;
      return utils::eq_any(oc, Bool, Int, Float, Frac, Cplx);
    }
    constexpr auto to_type_id(ir::op_code oc) noexcept
    {
      using eval::type_id;
      using enum ir::op_code;
      switch (oc)
      {
      case Bool:  return type_id::Bool;
      case Int:   return type_id::Int;
      case Float: return type_id::Float;
      case Frac:  return type_id::Fraction;
      case Cplx:  return type_id::Complex;
      }

      return type_id::Invalid;
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

  void ir_eval::try_load(const ir::vreg& reg) noexcept
  {
    auto regId = m_env.find_reg(m_curFrame, &reg);
    if (!regId)
      return;

    m_result = m_curFrame->value_for(*regId);
  }

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
    init_instr_ptr(*entry.begin());
  }

  void ir_eval::init_instr_ptr(const ir::instruction& instr) noexcept
  {
    m_instrPtr = &instr;
  }

  const ir::instruction* ir_eval::instr_ptr() const noexcept
  {
    return m_instrPtr;
  }

  void ir_eval::leave() noexcept
  {
    if (!m_curFrame)
    {
      m_instrPtr = nullptr;
      return;
    }

    m_instrPtr = detail::to_addr(m_curFrame->jump_back());
    m_env.remove_frame(m_curFrame);
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
    return get_value(*m_curFrame, op);
  }

  ir_eval::val_opt ir_eval::get_value(const eval::stack_frame& frame, const ir::operand& op) const noexcept
  {
    val_opt res{};
    if (op.is_value())
    {
      res.emplace(op.get_value());
    }
    else if (op.is_register())
    {
      auto&& reg = op.get_reg();
      const auto regId = get_reg(&frame, reg);
      res.emplace(frame.value_for(regId));
    }

    return res;
  }

  entity_id ir_eval::get_reg(const ir::vreg& reg) const noexcept
  {
    return get_reg(m_curFrame, reg);
  }

  entity_id ir_eval::get_reg(const eval::stack_frame* frame, const ir::vreg& reg) const noexcept
  {
    const auto regId = m_env.find_reg(frame, &reg);
    UTILS_ASSERT(regId);
    return regId.value_or(entity_id{});
  }

  void ir_eval::store_value(entity_id reg, const ir::operand& from) noexcept
  {
    store_value(*m_curFrame, reg, from);
  }

  void ir_eval::store_value(eval::stack_frame& frame, entity_id reg, const ir::operand& from) noexcept
  {
    auto fromVal = get_value(frame, from);
    UTILS_ASSERT(fromVal);
    store_value(frame, reg, std::move(*fromVal));
  }

  void ir_eval::store_value(entity_id reg, eval::value val) noexcept
  {
    store_value(*m_curFrame, reg, std::move(val));
  }

  void ir_eval::store_value(eval::stack_frame& frame, entity_id reg, eval::value val) noexcept
  {
    frame.store(reg, val);
    m_result = std::move(val);
  }

  entity_id ir_eval::alloc_new(entity_id op) noexcept
  {
    if (auto existing = m_env.find_reg(m_curFrame, op))
      return *existing;

    const auto regId = m_curFrame->allocate();
    m_env.map(m_curFrame, op, regId);
    return regId;
  }

  entity_id ir_eval::alloc_new(const ir::operand& op) noexcept
  {
    UTILS_ASSERT(op.is_register());
    auto&& target = op.get_reg();
    return alloc_new(entity_id{ &target });
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
    else if (opcode == Select)
      select();
    else if (opcode == Arr)
      alloc_array();
    else if (opcode == Append)
      append();
    else if (opcode == DynBind)
      dyn_bind();
    else if (detail::is_unary(opcode))
      unary(opcode);
    else if (detail::is_binary(opcode))
      binary(opcode);
    else if (detail::is_type(opcode))
      type(opcode);
  }

  void ir_eval::alloc() noexcept
  {
    auto&& instr = cur();
    auto&& allocRes = instr[0];
    alloc_new(allocRes);
  }

  void ir_eval::alloc_array() noexcept
  {
    auto&& instr = cur();
    auto&& arr = instr[0];
    auto&& sz = instr[1];
    UTILS_ASSERT(sz.is_index());
    const auto size = sz.get_index();
    const auto regId = alloc_new(arr);
    auto&& wrapper = m_valStore->alloc_wrapped(size);
    store_value(regId, eval::value::array(wrapper));
  }

  void ir_eval::append() noexcept
  {
    auto&& instr = cur();
    auto&& from = instr[0];
    auto&& to = instr[1];

    auto storedVal = get_value(from);
    UTILS_ASSERT(storedVal);

    auto arrWrp = eval::extract_array(get_value(to).value_or(eval::value{}));
    UTILS_ASSERT(arrWrp);

    auto&& arrData = arrWrp->data();
    arrData.add(std::move(*storedVal));
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
    m_env.map(m_curFrame, &target, *par);
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

  void ir_eval::dyn_bind() noexcept
  {
    auto&& instr = cur();
    auto&& res = instr[0];
    auto&& src = instr[1];
    auto&& name = instr[2];

    const auto regId = alloc_new(res);
    auto srcVal = get_value(src);
    UTILS_ASSERT(srcVal);
    auto func = eval::cast_value<eval::function_type>(*srcVal);
    if(!func)
    {
      // todo: error & abort
      return;
    }

    UTILS_ASSERT(name.is_name());
    auto memName = name.get_name();

    auto&& callable = *func;
    auto result = callable->lookup(memName);
    if(!result)
    {
      // todo: error & abort
      return;
    }

    store_value(regId, eval::value{ eval::function_type{ *result } });
  }

  void ir_eval::select() noexcept
  {
    auto&& instr = cur();
    auto&& res = instr[0];
    auto&& cond = instr[1];
    auto&& onTrue = instr[2];
    auto&& onFalse = instr[3];

    const auto regId = alloc_new(res);
    auto condVal = get_value(cond);
    UTILS_ASSERT(condVal);
    const auto testRes = eval::to_bool(*condVal);
    auto&& result = testRes ? onTrue : onFalse;
    store_value(regId, result);
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

  void ir_eval::type(ir::op_code oc) noexcept
  {
    const auto ti = detail::to_type_id(oc);
    auto&& instr = cur();
    auto&& res = instr[0];
    const auto regId = alloc_new(res);
    auto argIdx = 1u;
    const auto argSz = instr.operand_count() - argIdx;
    auto instance = eval::instantiate(ti, argSz, [&](eval::value& arg) noexcept
      {
        if (argIdx > argSz)
          return;

        auto argVal = get_value(instr[argIdx]);
        UTILS_ASSERT(argVal);
        arg = *argVal;
        ++argIdx;
      });

    store_value(regId, instance.value_or(eval::value{}));
  }

  bool ir_eval::call(entity_id regId, eval::value f, const ir::instruction& instr) noexcept
  {
    const auto argCount = instr.operand_count() - 2;
    auto callable = f.try_get<eval::function_type>();
    if (!callable)
    {
      return false;
    }

    auto&& func = *callable;
    if (func->param_count() != argCount)
    {
      return false;
    }

    auto prevFrame = m_curFrame;
    enter(*func);
    m_curFrame->attach_ret_val(regId);
    for (auto idx = op_count{ 2 }; idx < instr.operand_count(); ++idx)
    {
      auto arg = get_value(*prevFrame, instr[idx]);
      UTILS_ASSERT(arg);
      m_curFrame->add_arg(std::move(*arg));
    }

    return true;
  }

  bool ir_eval::call(entity_id regId, eval::array_wrapper& arr, const ir::instruction& instr) noexcept
  {
    auto [arrIt, addOk] = m_arrCalls.try_emplace(&arr, 0u, regId);
    auto&& arrIdx = arrIt->second.m_idx;

    if (const auto sz = arr.size(); addOk)
    {
      auto&& resW = m_valStore->alloc_wrapped(sz);
      store_value(regId, eval::value::array(resW));
    }
    else if (arrIdx >= sz)
    {
      m_arrCalls.erase(&arr);
      if (m_arrCalls.empty())
        m_instrPtr = instr.next();

      auto res = m_curFrame->value_for(regId);
      auto resArr = eval::extract_array(res);
      auto&& resData = resArr->data();
      for (auto&& elem : resData)
      {
        auto elemId = m_env.find_reg(m_curFrame, &elem);
        UTILS_ASSERT(elemId);
        elem = m_curFrame->value_for(*elemId);
      }

      resData.erase([](const eval::value& val) noexcept
        {
          auto arr = eval::extract_array(val);
          return arr && arr->size() == std::size_t{};
        });

      auto&& resWrp = m_valStore->wrap(resData);
      store_value(regId, eval::value::array(resWrp));
      return false;
    }

    auto allocElem = [&](eval::stack_frame& frame) noexcept
      {
        auto resStorage = frame.value_for(regId);
        auto resArr = eval::extract_array(resStorage);
        UTILS_ASSERT(resArr);
        auto&& underlying = resArr->data();
        underlying.add(eval::value{});
        return entity_id{ &(*underlying.rbegin()) };
      };

    for (auto it = std::next(arr.begin(), arrIdx); it != arr.end(); ++it)
    {
      if (auto subarr = eval::extract_array(*it))
      {
        auto existing = m_arrCalls.find(subarr);
        entity_id subReg{};
        if (existing == m_arrCalls.end())
        {
          const auto parentElem = allocElem(*m_curFrame);
          subReg = alloc_new(parentElem);
        }
        else
        {
          subReg = existing->second.m_callRes;
        }

        if (call(subReg, *subarr, instr))
          break;
      }

      ++arrIdx;
      auto prevFrame = m_curFrame;
      if (!call(regId, *it, instr))
        continue;

      const auto elemAddr = allocElem(*prevFrame);
      auto callFrame = m_curFrame;
      m_curFrame = prevFrame;
      const auto callRes = alloc_new(elemAddr);
      m_curFrame = callFrame;
      m_curFrame->attach_ret_val(callRes);
      m_curFrame->redirrect(&instr);
      break;
    }

    return true;
  }

  void ir_eval::call() noexcept
  {
    auto&& instr = cur();
    auto&& to = instr[0];
    auto&& f = instr[1];
    const auto regId = alloc_new(to);
    auto callable = get_value(f);
    UTILS_ASSERT(callable);
    if (auto arr = eval::extract_array(callable.value_or(eval::value{})))
    {
      call(regId, *arr, instr);
      return;
    }

    if (!call(regId, *callable, instr))
    {
      // todo: error & abort
      return;
    }
  }

  void ir_eval::ret()
  {
    auto&& instr = cur();
    auto&& op = instr[0];
    const auto retAddr = m_curFrame->ret_val();

    auto retVal = get_value(op);
    UTILS_ASSERT(retVal);

    auto retFrame = m_curFrame->prev();

    // Root
    if (!retFrame)
    {
      m_result = std::move(*retVal);
      leave();
      return;
    }

    store_value(*retFrame, retAddr, std::move(*retVal));
    leave();
  }
}