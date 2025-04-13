#include "cfg/ir/ir_function.hpp"
#include "cfg/ir/ir_instructions.hpp"

namespace tnac::ir
{
  // Special members

  function::~function() noexcept = default;

  function::function(name_t name, entity_id id, size_type paramCount, block_list blocks) noexcept :
    function{ name, id, paramCount, {}, std::move(blocks) }
  {}

  function::function(name_t name, entity_id id, size_type paramCount, function& owner, block_list blocks) noexcept :
    function{ name, id, paramCount, &owner, std::move(blocks) }
  {
    owner.add_child(*this);
  }

  function::function(name_t name, entity_id id, size_type paramCount, function* owner, block_list blocks) noexcept :
    node{ kind::Function },
    m_name{ name },
    m_owner{ owner },
    m_blocks{ std::move(blocks) },
    m_id{ id },
    m_paramCount{ paramCount }
  {}


  // Public members

  function::name_t function::name() const noexcept
  {
    return m_name;
  }

  entity_id function::id() const noexcept
  {
    return m_id;
  }

  bool function::is_loose() const noexcept
  {
    return m_loose;
  }

  function::size_type function::param_count() const noexcept
  {
    return m_paramCount;
  }

  const basic_block& function::entry() const noexcept
  {
    UTILS_ASSERT(m_entry);
    return *m_entry;
  }
  basic_block& function::entry() noexcept
  {
    return FROM_CONST(entry);
  }

  const function* function::owner_func() const noexcept
  {
    return m_owner;
  }
  function* function::owner_func() noexcept
  {
    return FROM_CONST(owner_func);
  }

  const function::child_list& function::children() const noexcept
  {
    return m_children;
  }
  function::child_list& function::children() noexcept
  {
    return FROM_CONST(children);
  }

  const function* function::lookup(string_t fn) const noexcept
  {
    auto found = m_childSt.find(fn);
    return found != m_childSt.end() ? found->second : nullptr;
  }
  function* function::lookup(string_t fn) noexcept
  {
    return FROM_CONST(lookup, fn);
  }

  const function::block_list& function::blocks() const noexcept
  {
    return m_blocks;
  }
  function::block_list& function::blocks() noexcept
  {
    return FROM_CONST(blocks);
  }

  basic_block& function::create_block(string_t name) noexcept
  {
    auto&& block = m_blocks.add(name, name, *this);
    if (!m_entry)
      m_entry = &block;

    return block;
  }

  void function::delete_block_tree(basic_block& root) noexcept
  {
    root.clear_instructions();
    for (auto out : root.outs())
    {
      auto&& target = out->outgoing();
      if(target.is_last_connection(root))
        delete_block_tree(target);
    }
    m_blocks.remove(root.name());
  }

  void function::add_child_name(function& child) noexcept
  {
    add_child_name(child.raw_name(), child);
  }


  // Private members

  string_t function::raw_name() const noexcept
  {
    auto parts = utils::split(m_name, ":"sv);
    return *parts.begin();
  }

  void function::add_child(function& child) noexcept
  {
    m_children.push_back(&child);
    add_child_name(child);
  }

  void function::add_child_name(string_t name, function& child) noexcept
  {
    m_childSt.try_emplace(name, &child);
  }

  void function::make_loose() noexcept
  {
    m_loose = true;
  }
}