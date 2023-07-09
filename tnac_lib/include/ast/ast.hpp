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
#include "general/ast_kinds.inl"
  };

  //
  // Base class for all AST nodes
  //
  class node
  {
  public:
    using kind = node_kind;
    using enum kind;

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
    // const version
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
    template <typename... KINDS> requires(utils::detail::all_same<kind, KINDS...>)
    auto is_any(KINDS... kinds) const noexcept
    {
      return ((is(kinds)) || ...);
    }

  protected:
    //
    // Assigns a parent. Called from non-terminal nodes which
    // can have children
    //
    void make_child_of(node* parent) noexcept;

    //
    // Sets itself as the parent of the given child node
    //
    void assume_ancestry(node* child) noexcept;

  private:
    node* m_parent{};
    kind m_kind{ Error };
  };

  inline auto get_id(const node& n) noexcept
  {
    return n.what();
  }

  //
  // Checks whether the target class inherits from ast::node
  //
  template <typename D>
  concept ast_node = std::is_base_of_v<ast::node, D>;

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

  public:
    //
    // Merges the given list of child nodes with the list it currently holds
    //
    void adopt(elem_list children) noexcept;

    //
    // Returns the current list of children
    // 
    // const version
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