#include "sema/symbol.hpp"
#include "ast/ast.hpp"
#include "ast/ast_decls.hpp"

namespace tnac::semantics
{
  // Base symbol

  symbol::~symbol() noexcept = default;

  symbol::symbol(kind k, ast::decl& decl, const scope& owner) noexcept :
    m_owner{ &owner },
    m_decl{ &decl },
    m_kind{ k }
  {
    decl.attach_symbol(*this);
  }

  sym_kind symbol::what() const noexcept
  {
    return m_kind;
  }

  const scope& symbol::owner_scope() const noexcept
  {
    return *m_owner;
  }

  const ast::decl& symbol::declarator() const noexcept
  {
    return *m_decl;
  }
  ast::decl& symbol::declarator() noexcept
  {
    return FROM_CONST(declarator);
  }

  string_t symbol::name() const noexcept
  {
    return declarator().name();
  }

  eval::value symbol::value() const noexcept
  {
    return m_value;
  }

  void symbol::eval_result(eval::value val) noexcept
  {
    m_value = val;
  }

  // Variable symbol

  variable::~variable() noexcept = default;

  variable::variable(const scope& owner, ast::decl& decl) noexcept :
    symbol{ kind::Variable, decl, owner }
  {}


  // Parameter symbol

  parameter::~parameter() noexcept = default;

  parameter::parameter(const scope& owner, ast::decl& decl) noexcept :
    symbol{ kind::Parameter, decl, owner }
  {}


  // Function symbol

  function::~function() noexcept = default;

  function::function(const scope& owner, ast::decl& decl) noexcept :
    symbol{ kind::Function, decl, owner }
  {}

}