#include "compiler/detail/name_repo.hpp"

namespace tnac::detail
{
  // Special members

  name_repo::~name_repo() noexcept = default;

  name_repo::name_repo() noexcept = default;


  // Public members

  name_repo::idx_cache name_repo::init_indicies() noexcept
  {
    return m_prefNames;
  }

  string_t name_repo::entry_block_name() noexcept
  {
    return "entry"sv;
  }

  string_t name_repo::ret_block_name() noexcept
  {
    return "ret"sv;
  }

  string_t name_repo::make_block_name(string_t prefix, string_t postfix) noexcept
  {
    auto name = m_plainNames.format("{}.{}."sv, prefix, postfix);
    return m_prefNames.next_indexed(name);
  }

  string_t name_repo::mangle_module_name(string_t original, std::size_t parCnt) noexcept
  {
    return mangle(original, {}, parCnt);
  }

  string_t name_repo::mangle_func_name(string_t original, const ir::function& owner, std::size_t parCnt) noexcept
  {
    return mangle(original, &owner, parCnt);
  }

  string_t name_repo::op_name(ir::op_code oc) noexcept
  {
    auto name = ir::instruction::opcode_str(oc);
    return m_prefNames.next_indexed(name);
  }


  // Private members

  string_t name_repo::mangle(string_t original, const ir::function* owner, std::size_t parCnt) noexcept
  {
    const auto pc = static_cast<param_count>(parCnt);
    return owner ?
      m_plainNames.format("{}:{}@{:X}"sv, original, pc, *owner->id()) :
      m_plainNames.format("{}:{}"sv, original, pc);
  }
}