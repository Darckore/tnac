//
// Symbol
//

#pragma once
#include "evaluator/value.hpp"
#include "ast/ast_decls.hpp"

namespace tnac::sema
{
  //
  // Symbol kinds
  //
  enum class sym_kind : std::uint8_t
  {
    Empty,

    Variable
  };

  //
  // Base symbol information object. Holds a declarator and a value it refers to
  //
  class symbol
  {
  public:
    using kind = sym_kind;
    using enum kind;

  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(symbol);

    virtual ~symbol() noexcept;

  protected:
    symbol(kind k, ast::decl& decl, eval::value val) noexcept;

  public:
    kind what() const noexcept;

    bool is_variable() const noexcept;

    const ast::decl& declarator() const noexcept;
    ast::decl& declarator() noexcept;

    string_t name() const noexcept;

  private:
    ast::decl* m_decl{};
    eval::value m_value{};
    kind m_kind{};
  };


  //
  // Symbol corresponding to a variable
  //
  class variable : public symbol
  {
  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(variable);

    virtual ~variable() noexcept;

  protected:
    variable(ast::decl& decl, eval::value val) noexcept;
  };
}