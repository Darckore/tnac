#include "sema/symbol.hpp"
#include "ast/ast.hpp"
#include "ast/ast_decls.hpp"

namespace tnac::semantics
{
  // Base symbol

  symbol::~symbol() noexcept = default;

  symbol::symbol(kind k, ast::decl& decl) noexcept :
    m_decl{ &decl },
    m_kind{ k }
  {
    decl.attach_symbol(*this);
  }

  sym_kind symbol::what() const noexcept
  {
    return m_kind;
  }

  bool symbol::is_variable() const noexcept
  {
    return what() == Variable;
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

  variable::variable(ast::decl& decl) noexcept :
    symbol{ kind::Variable, decl }
  {}

}