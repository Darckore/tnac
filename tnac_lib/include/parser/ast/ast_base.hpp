//
// Base AST
//

#pragma once
#include "parser/token.hpp"

namespace tnac::ast
{
  //
  // Kinds of ast nodes
  //
  enum class node_kind : std::uint16_t
  {
    Error,
    Root,
    Scope,
    Module,
    Import,

    // Expressions
    Literal,
    Identifier,
    Unary,
    Tail,
    Binary,
    Assign,
    Array,
    Paren,
    Abs,
    Typed,
    Call,
    Decl,
    Result,
    Ret,
    Cond,
    CondShort,
    Matcher,
    Pattern,
    Dot,

    // Decls,
    VarDecl,
    FuncDecl,
    ParamDecl
  };

  //
  // Base class for all AST nodes
  //
  class node
  {
  public:
    using kind = node_kind;
    using enum node_kind;

  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(node);

    virtual ~node() noexcept;

  protected:
    node(kind k) noexcept;

  public:
    //
    // Returns the parent node if one exists
    //
    const node* parent() const noexcept;

    //
    // Returns the parent node if one exists
    // 
    node* parent() noexcept;

    //
    // Returns the node's kind
    //
    kind what() const noexcept;

    //
    // Checks whether the node has the specified kind
    //
    bool is(kind k) const noexcept;

    //
    // Checks whether the node is of one of the specified kinds
    //
    template <typename... KINDS> requires(utils::all_same<kind, KINDS...>)
    auto is_any(KINDS... kinds) const noexcept
    {
      return ((is(kinds)) || ...);
    }

    //
    // Checks whether the current node is valid (doesn't contain errors)
    //
    bool is_valid() const noexcept;

    //
    // Climbs the tree until the specified kind of node is encountered
    // Returns nullptr is reached the root
    //
    template <kind K>
    auto climb() const noexcept
    {
      using res_type = decltype(utils::try_cast<K>(this));
      auto top = parent();
      while (top)
      {
        auto res = utils::try_cast<K>(top);
        if (res) return res;
        top = top->parent();
      }
      return res_type{};
    }

    //
    // Climbs the tree until the specified kind of node is encountered
    // Returns nullptr is reached the root
    // 
    template <kind K>
    auto climb() noexcept
    {
      return FROM_CONST(template climb<K>);
    }

  protected:
    //
    // Makes this node ivalid
    //
    void make_invalid() noexcept;

    //
    // Makes this node invalid if the given child is invalid
    //
    void make_invalid_if(node* child) noexcept;

    //
    // Assigns a parent. Called from non-terminal nodes which
    // can have children
    //
    void make_child_of(node* parent) noexcept;

    //
    // Sets itself as the parent of the given child node
    //
    void assume_ancestry(node* child) noexcept;

    //
    // Invalidates all parent nodes if invalid itself
    //
    void invalidate_parents() noexcept;

  private:
    node* m_parent{};
    kind m_kind{ Error };
    bool m_valid{ true };
  };

  //
  // Checks whether the target class inherits from ast::node
  //
  template <typename D>
  concept ast_node = std::derived_from<D, ast::node>;

  inline auto get_id(const ast_node auto& n) noexcept
  {
    return n.what();
  }


  //
  // Mixin base for easy type definitions
  //
  template <ast_node N>
  struct list
  {
    using elem = N;
    using pointer = elem*;
    using const_pointer = const elem*;
    using reference = elem&;
    using const_reference = const elem&;

    using elem_list = std::vector<pointer>;
  };

  class expr;

  //
  // A special root node. Contains a list of all expressions in the current scope
  //
  class scope : public node
  {
  public:
    using element = expr;
    using pointer = element*;
    using const_pointer = const element*;

    using elem_list = std::vector<pointer>;

  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(scope);

    virtual ~scope() noexcept;

  protected:
    scope(elem_list children) noexcept;

    explicit scope(kind k) noexcept;

  public:
    //
    // Merges the given list of child nodes with the list it currently holds
    //
    void adopt(elem_list children) noexcept;

    //
    // Returns the current list of children
    //
    const elem_list& children() const noexcept;

    //
    // Returns the current list of children
    // 
    elem_list& children() noexcept;

    //
    // Checks whether the current scope is global
    //
    bool is_global() const noexcept;

  private:
    elem_list m_children;
  };
}