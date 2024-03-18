#include "cfg/ir/ir_builder.hpp"

namespace tnac::ir
{
  // Special members

  builder::~builder() noexcept = default;

  builder::builder() noexcept = default;


  // Public members

  function& builder::make_module(entity_id id, fname_t name, par_size_t paramCount) noexcept
  {
    return make_function(id, {}, name, paramCount);
  }

  function& builder::make_function(entity_id id, function& owner, fname_t name, par_size_t paramCount) noexcept
  {
    return make_function(id, &owner, name, paramCount);
  }

  function* builder::find_function(entity_id id) noexcept
  {
    auto fIt = m_functions.find(id);
    return fIt != m_functions.end() ? &fIt->second : nullptr;
  }

  instruction& builder::add_instruction(basic_block& owner, op_code op) noexcept
  {
    auto&& newInstr = m_instructions.emplace_back(owner, op);
    owner.add_instruction(newInstr);
    return newInstr;
  }

  instruction& builder::add_var(basic_block& owner, instruction_list::iterator pos) noexcept
  {
    auto&& var = m_instructions.emplace_before(pos, owner, op_code::Alloc);
    if (pos == owner.begin())
      owner.add_instruction_front(var);

    return var;
  }

  vreg& builder::make_register(string_t name) noexcept
  {
    return m_regs.emplace_front(name);
  }
  vreg& builder::make_register(vreg::idx_type idx) noexcept
  {
    return m_regs.emplace_front(idx);
  }


  // Private members

  function& builder::make_function(entity_id id, function* owner, fname_t name, par_size_t paramCount) noexcept
  {
    auto blockIt = m_blocks.try_emplace(id);
    UTILS_ASSERT(blockIt.second);
    auto blocks = block_container{ blockIt.first->second };

    auto newIt = owner ?
      m_functions.try_emplace(id, name, id, paramCount, *owner, std::move(blocks)) :
      m_functions.try_emplace(id, name, id, paramCount, std::move(blocks));
    UTILS_ASSERT(newIt.second);

    return newIt.first->second;
  }
}