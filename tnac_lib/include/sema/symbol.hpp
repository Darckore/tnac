//
// Symbol
//

#pragma once
#include "exec/value/value.hpp"
#include "parser/ast/ast_decls.hpp"

namespace tnac::ast
{
  class scope;
  class decl;
}

namespace tnac::semantics
{
  //
  // Symbol kinds
  //
  enum class sym_kind : std::uint8_t
  {
    Variable,
    Parameter,
    Function
  };

  //
  // A structure representing a scope
  // Even though tnac has no scopes in the classical sense,
  // we need to keep track of where symbols are declared to disambiguate
  // between entities having the same name
  //
  struct scope
  {
    const scope* m_enclosing{};
    const ast::scope* m_node{};
  };

  //
  // Base symbol information object. Holds a declarator of an entity
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
    symbol(kind k, ast::decl& decl, const scope& owner) noexcept;

  public:
    //
    // Returns the symbol kind
    //
    kind what() const noexcept;

    //
    // Checks whether the symbol is of the specified kind
    //
    bool is(kind k) const noexcept;

    //
    // Checks whether the symbol is of one of the specified kinds
    //
    template <typename... KINDS> requires(utils::all_same<kind, KINDS...>)
      auto is_any(KINDS... kinds) const noexcept
    {
      return ((is(kinds)) || ...);
    }

    //
    // Returns the owner scope
    //
    const scope& owner_scope() const noexcept;

    //
    // Returns the declarator for which this symbol was created
    // 
    // const version
    //
    const ast::decl& declarator() const noexcept;

    //
    // Returns the declarator for which this symbol was created
    // 
    ast::decl& declarator() noexcept;

    //
    // Returns the entity name
    //
    string_t name() const noexcept;

    //
    // Returns the value associated with the entity
    //
    eval::value value() const noexcept;

    //
    // Assigns a value to the entity
    // The evaluator sets this
    //
    void eval_result(eval::value val) noexcept;

  private:
    const scope* m_owner{};
    ast::decl* m_decl{};
    eval::value m_value{};
    kind m_kind{};
  };

  namespace detail
  {
    template <typename T>
    concept sym = std::is_base_of_v<symbol, T>;
  }

  inline auto get_id(const symbol& sym) noexcept
  {
    return sym.what();
  }

  //
  // Symbol corresponding to a variable
  //
  class variable final : public symbol
  {
  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(variable);

    virtual ~variable() noexcept;

  protected:
    variable(const scope& owner, ast::decl& decl) noexcept;
  };


  //
  // Symbol corresponding to a parameter
  //
  class parameter final : public symbol
  {
  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(parameter);

    virtual ~parameter() noexcept;

  protected:
    parameter(const scope& owner, ast::decl& decl) noexcept;
  };


  //
  // Symbol corresponding to a function
  //
  class function final : public symbol
  {
  public:
    using decl_type  = ast::func_decl;
    using size_type  = decl_type::size_type;
    using param_list = decl_type::param_list;

  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(function);

    virtual ~function() noexcept;

  protected:
    function(const scope& owner, ast::decl& decl) noexcept;

  public:
    size_type param_count() const noexcept;

    const param_list& params() const noexcept;
    param_list& params() noexcept;

  private:
    const ast::func_decl& func_decl() const noexcept;
    ast::func_decl& func_decl() noexcept;
  };
}

TYPE_TO_ID_ASSOCIATION(tnac::semantics::variable,  tnac::semantics::sym_kind::Variable);
TYPE_TO_ID_ASSOCIATION(tnac::semantics::parameter, tnac::semantics::sym_kind::Parameter);
TYPE_TO_ID_ASSOCIATION(tnac::semantics::function,  tnac::semantics::sym_kind::Function);