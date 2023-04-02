#include "sema/symbol.hpp"

namespace tnac::sema
{
  // Base symbol

  symbol::~symbol() noexcept = default;

  symbol::symbol(kind k, ast::decl& decl, eval::value val) noexcept :
    m_decl{ &decl },
    m_value{ val },
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


  // Variable symbol

  variable::~variable() noexcept = default;

  variable::variable(ast::decl& decl, eval::value val) noexcept :
    symbol{ kind::Variable, decl, val }
  {}

}