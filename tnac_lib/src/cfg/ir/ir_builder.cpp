#include "cfg/ir/ir_builder.hpp"
#include "eval/value/type_impl.hpp"

namespace tnac::ir
{
  block_container builder::loose_module::dummy_container() noexcept
  {
    static block_map dummyBlocks;
    return block_container{ dummyBlocks };
  }
}

namespace tnac::ir
{
  // Special members

  builder::~builder() noexcept
  {
    m_instructions.clear();
    m_synthPhiNodes.clear();
  }

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

  function& builder::make_loose(entity_id id, fname_t name) noexcept
  {
    auto&& newFunc = m_looseModules.emplace_back(id, name);
    newFunc.m_module.make_loose();
    return newFunc.m_module;
  }

  instruction& builder::add_instruction(basic_block& owner, op_code op, size_type count, instruction_list::iterator pos) noexcept
  {
    auto&& newInstr = m_instructions.emplace_before(pos, owner, op, count);
    owner.add_instruction(newInstr);
    return newInstr;
  }

  instruction& builder::add_instruction(basic_block& owner, op_code op, instruction_list::iterator pos) noexcept
  {
    auto&& newInstr = m_instructions.emplace_before(pos, owner, op);
    owner.add_instruction(newInstr);
    return newInstr;
  }

  instruction& builder::add_var(basic_block& owner, instruction_list::iterator pos) noexcept
  {
    return add_alloc(owner, op_code::Alloc, pos);
  }

  instruction& builder::add_array(basic_block& owner, instruction_list::iterator pos) noexcept
  {
    return add_alloc(owner, op_code::Arr, pos);
  }

  vreg& builder::make_register(string_t name) noexcept
  {
    return m_regs.emplace_front(name, vreg::Local);
  }
  vreg& builder::make_register(vreg::idx_type idx) noexcept
  {
    return m_regs.emplace_front(idx, vreg::Local);
  }

  vreg& builder::make_global_register(string_t name) noexcept
  {
    return m_regs.emplace_front(name, vreg::Global);
  }
  vreg& builder::make_global_register(vreg::idx_type idx) noexcept
  {
    return m_regs.emplace_front(idx, vreg::Global);
  }

  edge& builder::make_edge(basic_block& from, basic_block& to, operand val) noexcept
  {
    return m_edges.emplace_back(from, to, std::move(val));
  }

  edge& builder::make_loose(basic_block& from, basic_block& to, operand val) noexcept
  {
    return m_looseEdges.emplace_back(edge::loose, from, to, std::move(val));
  }

  instruction& builder::synth_phi(basic_block& owner) noexcept
  {
    return m_synthPhiNodes.emplace_back(owner, op_code::Phi);
  }

  constant& builder::intern(vreg& reg, eval::array_type val) noexcept
  {
    const auto id = val->id();
    auto&& res = m_consts.emplace_back(reg, const_val{ std::move(val) });
    [[maybe_unused]] const auto emplaceOk = m_arrays.try_emplace(id, &res).second;
    UTILS_ASSERT(emplaceOk);
    return res;
  }

  builder::instruction_list& builder::instructions() noexcept
  {
    return m_instructions;
  }

  builder::edge_list& builder::edges() noexcept
  {
    return m_edges;
  }

  builder::const_list& builder::interned() noexcept
  {
    return m_consts;
  }

  constant* builder::interned(const eval::array_type& val) noexcept
  {
    auto item = m_arrays.find(val->id());
    if (item == m_arrays.end())
    {
      UTILS_ASSERT(false);
      return {};
    }
    return item->second;
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

  instruction& builder::add_alloc(basic_block& owner, op_code oc, instruction_list::iterator pos) noexcept
  {
    auto&& alloc = m_instructions.emplace_before(pos, owner, oc);
    auto ownerBeg = owner.begin();
    if (pos == ownerBeg || !ownerBeg)
      owner.add_instruction_front(alloc);

    return alloc;
  }
}