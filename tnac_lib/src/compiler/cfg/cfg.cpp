#include "compiler/cfg/cfg.hpp"

namespace tnac::comp
{
  // Special members

  cfg::~cfg() noexcept = default;

  cfg::cfg() noexcept :
    m_valVisitor{ m_valReg }
  {}

  cfg::operator bool() const noexcept
  {
    return static_cast<bool>(m_entry);
  }


  // Public members

  void cfg::enter_block(basic_block& block) noexcept
  {
    // No reason to enter the same block twice
    if (&block == m_entry)
      return;

    m_entryChain.push(m_entry);
    m_entry = &block;
  }

  void cfg::exit_block() noexcept
  {
    // We're at the top, original entry is preserved
    if (m_entryChain.empty())
      return;

    m_entry = m_entryChain.top();
    m_entryChain.pop();
  }

  basic_block& cfg::create(block_name name) noexcept
  {
    auto key = storage_key{ name };
    auto item = m_blocks.try_emplace(key, std::move(name));
    
    auto&& res = item.first->second;
    if (!m_entry)
      m_entry = &res;

    return res;
  }

  basic_block& cfg::entry() noexcept
  {
    UTILS_ASSERT(static_cast<bool>(*this));
    return *m_entry;
  }

  basic_block* cfg::find(storage_key name) noexcept
  {
    auto block = m_blocks.find(name);
    return block != m_blocks.end() ? &block->second : nullptr;
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
  }

  void cfg::consume_float(string_t raw) noexcept
  {
    m_valVisitor.visit_float_literal(raw);
  }

  void cfg::consume_true() noexcept
  {
    m_valVisitor.visit_bool_literal(true);
  }

  void cfg::consume_false() noexcept
  {
    m_valVisitor.visit_bool_literal(false);
  }

  void cfg::consume_i() noexcept
  {
    m_valVisitor.visit_i();
  }

  void cfg::consume_e() noexcept
  {
    m_valVisitor.visit_e();
  }

  void cfg::consume_pi() noexcept
  {
    m_valVisitor.visit_pi();
  }
}