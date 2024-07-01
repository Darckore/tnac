#include "cfg/cfg.hpp"

namespace tnac::ir
{
  // Special members

  cfg::~cfg() noexcept = default;

  cfg::cfg(builder& bld) noexcept :
    m_builder{ &bld }
  {}


  // Public members

  builder& cfg::get_builder() noexcept
  {
    return *m_builder;
  }

  function& cfg::declare_module(entity_id id, name_t name, size_type paramCount) noexcept
  {
    auto&& mod = m_builder->make_module(id, name, conv_param_count(paramCount));
    m_modules.push_back(&mod);
    return mod;
  }

  function& cfg::declare_function(entity_id id, function& owner, name_t name, size_type paramCount) noexcept
  {
    return m_builder->make_function(id, owner, name, conv_param_count(paramCount));
  }

  function* cfg::find_entity(entity_id id) noexcept
  {
    return m_builder->find_function(id);
  }

  edge& cfg::connect(basic_block& from, basic_block& to, operand val) noexcept
  {
    return m_builder->make_edge(from, to, val);
  }

  const cfg::instr_list& cfg::instructions() const noexcept
  {
    return m_builder->instructions();
  }
  cfg::instr_list& cfg::instructions() noexcept
  {
    return FROM_CONST(instructions);
  }

  const cfg::edge_list& cfg::edges() const noexcept
  {
    return m_builder->edges();
  }
  cfg::edge_list& cfg::edges() noexcept
  {
    return FROM_CONST(edges);
  }

  cfg::const_view cfg::interned_vals() const noexcept
  {
    return m_constants;
  }

  void cfg::intern(eval::stored_value val) noexcept
  {
    m_constants.emplace_back(std::move(val));
  }

  void cfg::intern(eval::value val) noexcept
  {
    eval::on_value(val, [&](auto v) noexcept
      {
        intern(eval::stored_value{ std::move(v) });
      });
  }

  // Private members

  function::size_type cfg::conv_param_count(size_type paramCount) noexcept
  {
    UTILS_ASSERT(paramCount <= std::numeric_limits<function::size_type>::max());
    return static_cast<function::size_type>(paramCount);
  }
}