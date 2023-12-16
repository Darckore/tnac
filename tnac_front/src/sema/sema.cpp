#include "sema/sema.hpp"
#include "parser/ast/ast.hpp"

namespace tnac
{
  sema::scope_guard::scope_guard(sema& s, semantics::scope* newScope) noexcept :
    m_sema{ &s },
    m_scope{ m_sema->m_curScope }
  {
    if (m_alive)
    {
      m_sema->m_curScope = newScope;
    }
  }

  sema::scope_guard::~scope_guard() noexcept
  {
    if (m_alive)
    {
      m_sema->m_curScope = m_scope;
    }
  }

  sema::scope_guard::scope_guard(scope_guard&& other) noexcept :
    m_sema{ other.m_sema },
    m_scope{ other.m_scope },
    m_alive{ other.m_alive }
  {
    other.m_scope = {};
    other.m_alive = false;
  }
  sema::scope_guard& sema::scope_guard::operator=(scope_guard&& other) noexcept
  {
    if (this != &other)
    {
      m_sema = other.m_sema;
      m_scope = other.m_scope;
      m_alive = other.m_alive;
      other.m_scope = {};
      other.m_alive = false;
    }
    return *this;
  }

  sema::scope_guard::operator bool() const noexcept
  {
    return m_alive;
  }

  semantics::scope* sema::scope_guard::current() noexcept
  {
    return m_sema->m_curScope;
  }
  semantics::scope* sema::scope_guard::prev() noexcept
  {
    return m_scope;
  }
}

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

  semantics::scope* sema::current_scope() noexcept
  {
    return m_curScope;
  }

  sema::scope_guard sema::assume_scope(semantics::scope& scope) noexcept
  {
    return { *this, &scope };
  }

  sema::sym_ptr sema::find(const token& tok, lookup_type type) noexcept
  {
    auto name = tok.value();
    if(!m_curScope || !m_curScope->is_deferred())
      return find(name, type);

    if (auto existing = find(name, Scoped))
      return existing;

    return &m_symTab.add_deferred_sym(name, m_curScope, tok.at());
  }

  sema::sym_ptr sema::find(string_t name, lookup_type type) noexcept
  {
    return (type == Scoped)
      ? m_symTab.scoped_lookup(name, m_curScope)
      : m_symTab.lookup(name, m_curScope);
  }

  void sema::visit_decl(ast::decl& decl) noexcept
  {
    if (!m_curScope)
    {
      UTILS_ASSERT(false);
      return;
    }

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
      UTILS_ASSERT(m_curScope->is_function());
      auto targetScope = m_curScope->enclosing();
      auto&& declParams = utils::cast<FuncDecl>(decl).params();
      auto&& sym = m_symTab.add_function(name, targetScope, make_params(declParams), loc, *m_curScope);
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
    if (!m_curScope)
    {
      UTILS_ASSERT(false);
      return;
    }

    UTILS_ASSERT(m_curScope->is_module());
    auto targetScope = m_curScope->enclosing();
    auto name = def.is_fake() ?
      contrive_name() :
      def.name();

    auto&& sym = m_symTab.add_module(name, targetScope, def.at(), *m_curScope);
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
      newScope.attach_symbol(*sym);
      m_curScope = &newScope;
    }
    else
    {
      m_curScope = &sym->referenced();
    }

    return *sym;
  }

  semantics::symbol& sema::visit_import_alias(ast::module_def& def) noexcept
  {
    return visit_import_alias(def.name(), def.at(), def.symbol());
  }

  semantics::symbol& sema::visit_import_alias(const token& id, semantics::module_sym& src) noexcept
  {
    return visit_import_alias(id.value(), id.at(), src);
  }

  sema::scope_guard sema::try_resolve_scope(ast::expr& expr) noexcept
  {
    auto found = m_curScope;
    if (auto sym = extract_sym(expr))
    {
      using enum semantics::sym_kind;
      if (sym->is_any(Module, Function))
        found = &utils::cast<Function>(*sym).own_scope();
      else if (auto sr = utils::try_cast<ScopeRef>(sym))
        found = &sr->referenced();
      else
        found = &m_symTab.add_scope(m_curScope, semantics::scope::Deferred);
    }
    else
    {
      found = &m_symTab.add_scope(m_curScope, semantics::scope::Deferred);
    }
    return { *this, found };
  }

  string_t sema::contrive_name() noexcept
  {
    return m_generatedNames.next_indexed("`__anon_entity__"sv);
  }

  string_t sema::contrive_func_name() noexcept
  {
    return m_generatedNames.next_indexed("`__anon_function__"sv);
  }

  // Private members

  semantics::symbol* sema::extract_sym(ast::expr& expr) const noexcept
  {
    if (auto par = utils::try_cast<ast::paren_expr>(&expr))
      return extract_sym(par->internal_expr());

    if (auto dot = utils::try_cast<ast::dot_expr>(&expr))
      return extract_sym(dot->accessor());

    if (auto id = utils::try_cast<ast::id_expr>(&expr))
      return &id->symbol();

    return {};
  }

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

  semantics::symbol& sema::visit_import_alias(string_t name, loc_t at, semantics::module_sym& src) noexcept
  {
    return m_symTab.add_scope_ref(name, m_curScope, at, src.own_scope());
  }

}