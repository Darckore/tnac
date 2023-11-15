#include "sema/sema.hpp"
#include "parser/ast/ast.hpp"

namespace tnac
{
  // Special members

  sema::~sema() noexcept = default;

  sema::sema() noexcept = default;

  // Public members

  void sema::open_scope(semantics::scope_kind kind) noexcept
  {
    m_curScope = &m_symTab.add_scope(m_curScope, kind);
  }

  void sema::close_scope() noexcept
  {
    if(m_curScope)
      m_curScope = m_curScope->enclosing();
  }

  sema::sym_ptr sema::find(string_t name, bool currentOnly /*= false*/) noexcept
  {
    return currentOnly
      ? m_symTab.scoped_lookup(name, m_curScope)
      : m_symTab.lookup(name, m_curScope);
  }

  void sema::visit_decl(ast::decl& decl) noexcept
  {
    using enum ast::node_kind;
    auto name = decl.name();
    auto loc = decl.pos().at();
    switch (decl.what())
    {
    case VarDecl:
    {
      auto&& var = m_symTab.add_variable(name, m_curScope, loc);
      decl.attach_symbol(var);
    }
    break;

    case ParamDecl:
    {
      decl.attach_symbol(m_symTab.add_parameter(name, m_curScope, loc));
    }
    break;

    case FuncDecl:
    {
      using namespace semantics;
      UTILS_ASSERT(m_curScope && m_curScope->is_function());
      auto targetScope = m_curScope->enclosing();
      auto&& declParams = utils::cast<FuncDecl>(decl).params();
      function::param_list params;
      params.reserve(declParams.size());
      for (auto&& param : declParams)
      {
        auto paramSym = utils::try_cast<parameter>(&param->symbol());
        UTILS_ASSERT(paramSym);
        params.emplace_back(paramSym);
      }
      auto&& sym = m_symTab.add_function(name, targetScope, std::move(params), loc);
      decl.attach_symbol(sym);
      m_curScope->attach_symbol(sym);
    }
    break;

    default:
      UTILS_ASSERT(false);
      break;
    }
  }

  void sema::visit_module_def(ast::module_def& def) noexcept
  {
    UTILS_ASSERT(m_curScope && m_curScope->is_module());
    auto targetScope = m_curScope->enclosing();
    auto name = def.is_fake() ?
      contrive_name() :
      def.name();
    auto&& sym = m_symTab.add_module(name, targetScope, def.at());
    def.attach_symbol(sym);
    m_curScope->attach_symbol(sym);
  }

  void sema::visit_module_entry(ast::module_def& def, module_params params, loc_t at) noexcept
  {
    def.attach_params(std::move(params));
    def.override_loc(at);
  }

  string_t sema::contrive_name() noexcept
  {
    static constexpr auto namePrefix{ "`__anon_entity__"sv };
    const auto nameIdx = m_generatedNames.size();
    auto entry = m_generatedNames.emplace(buf_t{ namePrefix } + std::to_string(nameIdx));
    return *entry.first;
  }

}