#include "parser/ast/ast_decls.hpp"

namespace tnac::ast
{
  // Base declarator

  decl::~decl() noexcept = default;

  decl::decl(kind k, const token& id, const token& pos, node* def) noexcept :
    node{ k },
    m_def{ def },
    m_id{ id },
    m_pos{ pos }
  {
    assume_ancestry(m_def);
  }

  decl::decl(kind k, const token& id, node* def) noexcept :
    decl{ k, id, id, def }
  {}

  void decl::attach_symbol(semantics::symbol& sym) noexcept
  {
    m_symbol = &sym;
  }

  string_t decl::name() const noexcept
  {
    return m_id.value();
  }

  const node* decl::definition() const noexcept
  {
    return m_def;
  }
  node* decl::definition() noexcept
  {
    return FROM_CONST(definition);
  }

  const token& decl::pos() const noexcept
  {
    return m_pos;
  }

  const semantics::symbol& decl::symbol() const noexcept
  {
    UTILS_ASSERT(static_cast<bool>(m_symbol));
    return *m_symbol;
  }
  semantics::symbol& decl::symbol() noexcept
  {
    return FROM_CONST(symbol);
  }


  // Decl Expression

  decl_expr::~decl_expr() noexcept = default;

  decl_expr::decl_expr(decl& d) noexcept :
    expr{ kind::Decl, d.pos() },
    m_decl{ &d }
  {
    assume_ancestry(m_decl);
  }

  const decl& decl_expr::declarator() const noexcept
  {
    return *m_decl;
  }
  decl& decl_expr::declarator() noexcept
  {
    return FROM_CONST(declarator);
  }


  // Variable decl

  var_decl::~var_decl() noexcept = default;

  var_decl::var_decl(const token& var, expr& initialiser) noexcept :
    decl{ kind::VarDecl, var, &initialiser }
  {}

  const expr& var_decl::initialiser() const noexcept
  {
    return utils::cast<expr>(*definition());
  }
  expr& var_decl::initialiser() noexcept
  {
    return FROM_CONST(initialiser);
  }


  // Parameter decl

  param_decl::~param_decl() noexcept = default;

  param_decl::param_decl(const token& paramName, expr* opt) noexcept :
    decl{ kind::ParamDecl, paramName, opt }
  {}


  // Function decl

  func_decl::~func_decl() noexcept = default;

  func_decl::func_decl(const token& func, const token& pos, scope& def, param_list params) noexcept :
    decl{ kind::FuncDecl, func, pos, &def },
    m_params{ std::move(params) }
  {
    for (auto p : m_params)
      assume_ancestry(p);
  }

  const func_decl::param_list& func_decl::params() const noexcept
  {
    return m_params;
  }
  func_decl::param_list& func_decl::params() noexcept
  {
    return FROM_CONST(params);
  }

  func_decl::size_type func_decl::param_count() const noexcept
  {
    return params().size();
  }

  const scope& func_decl::body() const noexcept
  {
    return utils::cast<scope>(*definition());
  }
  scope& func_decl::body() noexcept
  {
    return FROM_CONST(body);
  }
}