#include "ast/ast_decls.hpp"

namespace tnac::ast
{
  // Base declarator

  decl::~decl() noexcept = default;

  decl::decl(kind k, const token& id, expr& def) noexcept :
    node{ k },
    m_id{ id },
    m_def{ &def }
  {
    assume_ancestry(m_def);
  }

  void decl::attach_symbol(const semantics::symbol& sym) noexcept
  {
    m_symbol = &sym;
  }

  string_t decl::name() const noexcept
  {
    return m_id.m_value;
  }

  const expr& decl::definition() const noexcept
  {
    return *m_def;
  }
  expr& decl::definition() noexcept
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
    decl{ kind::VarDecl, var, initialiser }
  {}
}