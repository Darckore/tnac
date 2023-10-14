#include "compiler/cfg/cfg.hpp"

namespace tnac::comp
{
  // Special members

  cfg::~cfg() noexcept = default;

  cfg::cfg() noexcept :
    m_valVisitor{ m_valReg }
  {
  }


  // Public members(Basic blocks)

  void cfg::enter_block(basic_block& block) noexcept
  {
    // No reason to enter the same block twice
    if (&block == m_currentBlock)
      return;

    m_entryChain.push(m_currentBlock);
    m_currentBlock = &block;
  }

  void cfg::exit_block() noexcept
  {
    // We're at the top, original entry is preserved
    if (m_entryChain.empty())
      return;

    m_currentBlock = m_entryChain.top();
    m_entryChain.pop();
  }

  basic_block& cfg::create_block(block_name name) noexcept
  {
    auto key = storage_key{ name };
    auto item = m_blocks.try_emplace(key, std::move(name), current_func());
    
    auto&& res = item.first->second;
    if (!m_currentBlock)
      m_currentBlock = &res;

    return res;
  }

  basic_block& cfg::current_block() noexcept
  {
    return *m_currentBlock;
  }

  basic_block* cfg::find_block(storage_key name) noexcept
  {
    auto res = m_blocks.find(name);
    return res != m_blocks.end() ? &res->second : nullptr;
  }


  // Public members(Functions)

  func& cfg::create_function(func_name name) noexcept
  {
    auto key = storage_key{ name };
    auto item = m_functions.try_emplace(key, std::move(name), m_currentFunction);
    auto&& res = item.first->second;
    m_currentFunction = &res;

    return res;
  }

  func* cfg::find_func(storage_key name) noexcept
  {
    auto res = m_functions.find(name);
    return res != m_functions.end() ? &res->second : nullptr;
  }

  func& cfg::current_func() noexcept
  {
    return *m_currentFunction;
  }

  void cfg::end_function() noexcept
  {
    if (m_currentFunction)
      m_currentFunction = m_currentFunction->parent();
  }


  // Public members(Expressions)

  void cfg::consume_unary(eval::val_ops opCode) noexcept
  {
    using enum eval::val_ops;
    switch (opCode)
    {
    case LogicalNot: break;
    case LogicalIs: break;
    case UnaryPlus: break;
    case UnaryNegation: break;
    case UnaryBitwiseNot: break;

    default: break;
    }
  }

  void cfg::consume_binary(eval::val_ops opCode) noexcept
  {
    utils::unused(opCode);
  }

  // Public members(Literals)

  void cfg::consume_int(string_t raw, int base) noexcept
  {
    m_valVisitor.visit_int_literal(raw, base);
    emit_constant(eval::type_id::Int);
  }

  void cfg::consume_float(string_t raw) noexcept
  {
    m_valVisitor.visit_float_literal(raw);
    emit_constant(eval::type_id::Float);
  }

  void cfg::consume_true() noexcept
  {
    m_valVisitor.visit_bool_literal(true);
    emit_constant(eval::type_id::Bool);
  }

  void cfg::consume_false() noexcept
  {
    m_valVisitor.visit_bool_literal(false);
    emit_constant(eval::type_id::Bool);
  }

  void cfg::consume_i() noexcept
  {
    m_valVisitor.visit_i();
    emit_constant(eval::type_id::Complex);
  }

  void cfg::consume_e() noexcept
  {
    m_valVisitor.visit_e();
    emit_constant(eval::type_id::Float);
  }

  void cfg::consume_pi() noexcept
  {
    m_valVisitor.visit_pi();
    emit_constant(eval::type_id::Float);
  }


  // Private members

  void cfg::emit_constant(eval::type_id type) noexcept
  {
    auto value  = m_valVisitor.fetch_next();
    auto irNode = ir::operation{ ir::op_code::Constant, eval::size_of(type) };
    utils::unused(irNode, value, type);
  }

}