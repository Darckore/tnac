#include "sema/sema.hpp"
#include "sema/symbol.hpp"
#include "parser/ast/ast_nodes.hpp"

namespace tnac
{
  // Special members

  sema::~sema() noexcept = default;

  sema::sema() noexcept = default;

  // Public members

  void sema::open_scope(const ast::scope& scopeNode) noexcept
  {
    m_curScope = &m_symTab.add_scope(&scopeNode, m_curScope);
  }

  void sema::close_scope() noexcept
  {
    if(m_curScope)
      m_curScope = m_curScope->m_enclosing;
  }

  sema::sym_ptr sema::find(string_t name, bool currentOnly /*= false*/) noexcept
  {
    return currentOnly
      ? m_symTab.scoped_lookup(name, m_curScope)
      : m_symTab.lookup(name, m_curScope);
  }

  void sema::visit_decl(ast::decl& decl) noexcept
  {
    using enum ast::node::kind;
    switch (decl.what())
    {
    case VarDecl:
    {
      auto&& var = m_symTab.add_variable(decl, m_curScope);
      if (m_varCallback)
        m_varCallback(var);
    }
      break;

    case ParamDecl:
    {
      m_symTab.add_parameter(decl, m_curScope);
    }
      break;

    case FuncDecl:
    {
      UTILS_ASSERT(static_cast<bool>(m_curScope));
      auto targetScope = m_curScope->m_enclosing;
      m_symTab.add_function(decl, targetScope);
    }
    break;

    default:
      break;
    }
  }

  token sema::contrive_name() noexcept
  {
    static constexpr auto namePrefix{ "`__anon_entity__"sv };
    const auto nameIdx = m_generatedNames.size();
    auto entry = m_generatedNames.emplace(buf_t{ namePrefix } + std::to_string(nameIdx));
    return { *entry.first, token::Identifier };
  }

}