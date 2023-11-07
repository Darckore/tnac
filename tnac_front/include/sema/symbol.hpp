//
// Symbol
//

#pragma once
#include "parser/ast/ast_decls.hpp"

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
  };

  //
  // Base symbol information object. Holds information on an entity
  //
  class symbol
  {
  public:
    using kind   = sym_kind;
    using name_t = string_t;
    using enum sym_kind;

  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(symbol);

    virtual ~symbol() noexcept;

  protected:
    symbol(kind k, name_t name, const scope& owner) noexcept;

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
    // Returns the entity name
    //
    string_t name() const noexcept;

  private:
    const scope* m_owner{};
    name_t m_name;
    kind m_kind{};
  };

  namespace detail
  {
    template <typename T>
    concept sym = std::derived_from<T, symbol>;
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
    variable(const scope& owner, name_t name) noexcept;
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
    parameter(const scope& owner, name_t name) noexcept;
  };


  //
  // Symbol corresponding to a function
  //
  class function final : public symbol
  {
  public:
    using param_list = std::vector<parameter*>;
    using size_type  = param_list::size_type;

  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(function);

    virtual ~function() noexcept;

  protected:
    function(const scope& owner, name_t name, param_list params) noexcept;

  public:
    size_type param_count() const noexcept;

    const param_list& params() const noexcept;
    param_list& params() noexcept;

  private:
    param_list m_params;
  };
}

TYPE_TO_ID_ASSOCIATION(tnac::semantics::variable,  tnac::semantics::sym_kind::Variable);
TYPE_TO_ID_ASSOCIATION(tnac::semantics::parameter, tnac::semantics::sym_kind::Parameter);
TYPE_TO_ID_ASSOCIATION(tnac::semantics::function,  tnac::semantics::sym_kind::Function);