//
// Symbol
//

#pragma once
#include "evaluator/value.hpp"

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
    Variable
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
    symbol(kind k, ast::decl& decl) noexcept;

  public:
    kind what() const noexcept;

    bool is_variable() const noexcept;

    const ast::decl& declarator() const noexcept;
    ast::decl& declarator() noexcept;

    string_t name() const noexcept;

    eval::value value() const noexcept;

    void eval_result(eval::value val) noexcept;

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
    variable(ast::decl& decl) noexcept;
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
  static constexpr auto kind_from_sym = detail::kind_from_sym<S>::value;

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