//
// Symbol
//

#pragma once
#include "evaluator/value.hpp"
#include "ast/ast_decls.hpp"

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
    using size_type = ast::func_decl::size_type;

  private:
    friend class sym_table;

  public:
    CLASS_SPECIALS_NONE(function);

    virtual ~function() noexcept;

  protected:
    function(const scope& owner, ast::decl& decl) noexcept;

  public:
    size_type param_count() const noexcept;

  private:
    const ast::func_decl& func_decl() const noexcept;
    ast::func_decl& func_decl() noexcept;
  };


  namespace detail
  {
    template <typename T>
    concept sym = std::is_base_of_v<symbol, T>;

    template <sym S> struct kind_from_sym;
    template <sym_kind K> struct sym_from_kind;

    template <>
    struct kind_from_sym<variable>
    {
      static constexpr auto value = sym_kind::Variable;
    };
    template <>
    struct sym_from_kind<sym_kind::Variable>
    {
      using type = variable;
    };

    template <>
    struct kind_from_sym<parameter>
    {
      static constexpr auto value = sym_kind::Parameter;
    };
    template <>
    struct sym_from_kind<sym_kind::Parameter>
    {
      using type = parameter;
    };

    template <>
    struct kind_from_sym<function>
    {
      static constexpr auto value = sym_kind::Function;
    };
    template <>
    struct sym_from_kind<sym_kind::Function>
    {
      using type = function;
    };

    template <typename From, typename To> struct sym_caster;

    template <sym From, sym To> requires (std::is_const_v<From>)
    struct sym_caster<From, To>
    {
      using type = const To*;
    };

    template <sym From, sym To> requires (!std::is_const_v<From>)
    struct sym_caster<From, To>
    {
      using type = To*;
    };

  }

  template <detail::sym S>
  inline constexpr auto kind_from_sym = detail::kind_from_sym<S>::value;

  template <sym_kind K>
  using sym_from_kind = detail::sym_from_kind<K>::type;

  template <detail::sym To>
  auto sym_cast(const symbol* sym) noexcept
  {
    using res_t = detail::sym_caster<const symbol, To>::type;
    static constexpr auto K = kind_from_sym<To>;
    if (!sym || sym->what() != K)
      return res_t{};

    return static_cast<res_t>(sym);
  }

  template <detail::sym To>
  auto sym_cast(symbol* sym) noexcept
  {
    auto castRes = sym_cast<To>(static_cast<const symbol*>(sym));
    return const_cast<To*>(castRes);
  }
}