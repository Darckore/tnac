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
      UTILS_ASSERT(m_curScope && m_curScope->is_function());
      auto targetScope = m_curScope->enclosing();
      auto&& declParams = utils::cast<FuncDecl>(decl).params();
      auto&& sym = m_symTab.add_function(name, targetScope, make_params(declParams), loc);
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

  void sema::visit_module_entry(ast::module_def& def, ast_params params, loc_t at) noexcept
  {
    def.attach_params(std::move(params));
    def.override_loc(at);

    auto&& sym = def.symbol();
    sym.attach_params(make_params(def.params()));
    sym.override_location(at);
  }

  semantics::symbol& sema::visit_import_component(const token& id) noexcept
  {
    auto name = id.value();
    auto loc  = id.at();
    auto sym = utils::try_cast<semantics::scope_ref>(m_symTab.scoped_lookup(name, m_curScope));
    if (!sym)
    {
      auto&& newScope = m_symTab.add_scope(m_curScope, semantics::scope::Block);
      sym = &m_symTab.add_scope_ref(name, m_curScope, loc, newScope);
      m_curScope = &newScope;
    }
    else
    {
      m_curScope = &sym->referenced();
    }

    return *sym;
  }

  string_t sema::contrive_name() noexcept
  {
    static constexpr auto namePrefix{ "`__anon_entity__"sv };
    const auto nameIdx = m_generatedNames.size();
    auto entry = m_generatedNames.emplace(buf_t{ namePrefix } + std::to_string(nameIdx));
    return *entry.first;
  }


  // Private members

  sema::symbol_params sema::make_params(const ast_params& src) const noexcept
  {
    symbol_params params;
    params.reserve(src.size());
    for (auto&& param : src)
    {
      auto paramSym = utils::try_cast<semantics::parameter>(&param->symbol());
      UTILS_ASSERT(paramSym);
      params.emplace_back(paramSym);
    }

    return params;
  }

}