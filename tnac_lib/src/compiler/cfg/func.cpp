#include "compiler/cfg/func.hpp"

namespace tnac::comp
{
  // Special members

  func::~func() noexcept = default;

  func::func(name_t name, func* parent) noexcept :
    m_name{ std::move(name) },
    m_parent{ parent }
  {
    create_block("entry"s);
  }


  // Public members

  string_t func::name() const noexcept
  {
    return m_name;
  }

  const func* func::parent() const noexcept
  {
    return m_parent;
  }

  func* func::parent() noexcept
  {
    return FROM_CONST(parent);
  }

  basic_block& func::create_block(name_t name) noexcept
  {
    auto key = storage_key{ name };
    auto res = m_blocks.try_emplace(key, std::move(name), *this);
    auto block = &res.first->second;
    if(!m_entry)
      m_entry = block;

    enter_block(*block);
    return *block;
  }

  void func::enter_block(basic_block& block) noexcept
  {
    m_cur = &block;
  }


  const basic_block& func::entry() const noexcept
  {
    return *m_entry;
  }
  basic_block& func::entry() noexcept
  {
    return FROM_CONST(entry);
  }

  const basic_block& func::current_block() const noexcept
  {
    return *m_cur;
  }
  basic_block& func::current_block() noexcept
  {
    return FROM_CONST(current_block);
  }

  const environment& func::env() const noexcept
  {
    return m_env;
  }
  environment& func::env() noexcept
  {
    return FROM_CONST(env);
  }
}