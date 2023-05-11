#include "ast/ast_decls.hpp"

namespace tnac::ast
{
  // Base declarator

  decl::~decl() noexcept = default;

  decl::decl(kind k, const token& id, node* def) noexcept :
    node{ k },
    m_id{ id },
    m_def{ def }
  {
    assume_ancestry(m_def);
  }

  void decl::attach_symbol(semantics::symbol& sym) noexcept
  {
    m_symbol = &sym;
  }

  string_t decl::name() const noexcept
  {
    return m_id.m_value;
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
    return m_id;
  }

  const semantics::symbol& decl::symbol() const noexcept
  {
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
    return static_cast<const expr&>(*definition());
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

  func_decl::func_decl(const token& func, scope& def, param_list params) noexcept :
    decl{ kind::FuncDecl, func, &def }
  {}
}