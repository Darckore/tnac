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
    return "return"sv;
  }

  string_t name_repo::make_block_name(string_t prefix, string_t postfix) noexcept
  {
    auto name = m_plainNames.format("{}.{}."sv, prefix, postfix);
    return m_prefNames.next_indexed(name);
  }

  string_t name_repo::mangle_module_name(semantics::module_sym& sym, std::size_t parCnt) noexcept
  {
    std::vector<string_t> parts;
    auto symScope = &sym.owner_scope();
    while (symScope)
    {
      auto scopeRef = symScope->to_scope_ref();
      if (scopeRef)
      {
        parts.push_back(scopeRef->name());
      }
      else if (auto mod = symScope->to_module())
      {
        parts.push_back(mod->name());
      }

      symScope = symScope->enclosing();
    }

    buf_t namePref;
    for (auto part : parts | views::reverse)
    {
      namePref.append(part);
      namePref.push_back('.');
    }

    return !namePref.empty() ?
      m_plainNames.format("{}{}:{}"sv, std::move(namePref), sym.name(), parCnt):
      m_plainNames.format("{}:{}"sv, sym.name(), parCnt);
  }

  string_t name_repo::mangle_func_name(string_t original, const ir::function& owner, std::size_t parCnt) noexcept
  {
    return m_plainNames.format("{}:{}@{:X}"sv, original, parCnt, *owner.id());
  }

  string_t name_repo::op_name(ir::op_code oc) noexcept
  {
    auto name = ir::instruction::opcode_str(oc);
    return m_prefNames.next_indexed(name);
  }

}