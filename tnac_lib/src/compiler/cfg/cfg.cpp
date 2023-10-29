#include "compiler/cfg/cfg.hpp"

namespace tnac::comp
{
  // Special members

  cfg::~cfg() noexcept = default;

  cfg::cfg() noexcept :
    m_valVisitor{ m_valReg }
  {}


  // Public members(Functions)

  func& cfg::create_function(func_name name) noexcept
  {
    auto key = storage_key{ name };
    auto item = m_functions.try_emplace(key, std::move(name), m_currentFunction);
    auto&& res = item.first->second;
    m_currentFunction = &res;

    return res;
  }

  func* cfg::find_function(storage_key name) noexcept
  {
    auto res = m_functions.find(name);
    return res != m_functions.end() ? &res->second : nullptr;
  }

  void cfg::end_function() noexcept
  {
    if (!m_currentFunction)
      return;

    if (auto parent = m_currentFunction->parent())
      m_currentFunction = parent;
  }


  // Public members(Expressions)

  void cfg::consume_unary(eval::val_ops opCode) noexcept
  {
    using enum eval::val_ops;
    using enum ir::op_code;

    if (utils::eq_any(opCode, UnaryPlus, UnaryNegation))
    {
      const auto code    = (opCode == UnaryPlus) ? Add : Sub;
      const auto [saveTo, operand ] = extract_unary();
      current_block().add_binary(code, saveTo, {}, operand);
      return;
    }

    ir::op_code code{};
    switch (opCode)
    {
    case LogicalNot:      code = Not; break;
    case LogicalIs:       code = Is;  break;
    case UnaryBitwiseNot: code = Inv; break;
    case AbsoluteValue:   code = Abs; break;

    default: return;
    }

    const auto [saveTo, operand] = extract_unary();
    current_block().add_unary(code, saveTo, operand);
  }

  void cfg::consume_binary(eval::val_ops opCode) noexcept
  {
    using enum eval::val_ops;
    using enum ir::op_code;

    ir::op_code code{};
    switch (opCode)
    {
    case Addition:       code = Add;    break;
    case Subtraction:    code = Sub;    break;
    case Multiplication: code = Mul;    break;
    case Division:       code = Div;    break;
    case Modulo:         code = Mod;    break;
    case BitwiseAnd:     code = And;    break;
    case BitwiseXor:     code = Xor;    break;
    case BitwiseOr:      code = Or;     break;
    case BinaryPow:      code = Pow;    break;
    case BinaryRoot:     code = Root;   break;
    case RelLess:        code = Less;   break;
    case RelLessEq:      code = LessEq; break;
    case RelGr:          code = Gr;     break;
    case RelGrEq:        code = GrEq;   break;
    case Equal:          code = Eq;     break;
    case NEqual:         code = NEq;    break;

    default: return;
    }

    const auto [saveTo, left, right] = extract_binary();
    current_block().add_binary(code, saveTo, left, right);
  }


  // Public members(Literals)

  void cfg::consume_int(string_t raw, int base) noexcept
  {
    m_valVisitor.visit_int_literal(raw, base);
    emit_constant();
  }

  void cfg::consume_float(string_t raw) noexcept
  {
    m_valVisitor.visit_float_literal(raw);
    emit_constant();
  }

  void cfg::consume_true() noexcept
  {
    m_valVisitor.visit_bool_literal(true);
    emit_constant();
  }

  void cfg::consume_false() noexcept
  {
    m_valVisitor.visit_bool_literal(false);
    emit_constant();
  }

  void cfg::consume_i() noexcept
  {
    m_valVisitor.visit_i();
    emit_constant();
  }

  void cfg::consume_e() noexcept
  {
    m_valVisitor.visit_e();
    emit_constant();
  }

  void cfg::consume_pi() noexcept
  {
    m_valVisitor.visit_pi();
    emit_constant();
  }


  // Private members

  cfg::unary_regs cfg::extract_unary() noexcept
  {
    auto&& curEnv = env();
    const auto operand = curEnv.pop_register();
    const auto saveTo  = curEnv.next_register();
    return { saveTo, operand };
  }

  cfg::binary_regs cfg::extract_binary() noexcept
  {
    auto&& curEnv = env();
    const auto right  = curEnv.pop_register();
    const auto left   = curEnv.pop_register();
    const auto saveTo = curEnv.next_register();
    return { saveTo, left, right };
  }

  void cfg::emit_constant() noexcept
  {
    auto operand = m_valVisitor.fetch_next();
    current_block().add_constant(env().next_register(), *operand);
  }

  func& cfg::current_func() noexcept
  {
    return *m_currentFunction;
  }

  basic_block& cfg::current_block() noexcept
  {
    return current_func().current_block();
  }

  environment& cfg::env() noexcept
  {
    return current_func().env();
  }

}