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
    node(node* parent, kind k) noexcept;

  public:
    const node* parent() const noexcept;
    node* parent() noexcept;

  protected:
    void make_child_of(node* parent) noexcept;

    void assume_ancestry(node* child) noexcept;

  private:
    node* m_parent{};
    kind m_kind{ Error };
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
    scope(node* parent, elem_list children) noexcept;

  public:
    void adopt(elem_list children) noexcept;

  private:
    elem_list m_children;
  };

  //
  // Checks whether the target class inherits from
  //
  template <typename D>
  concept ast_node = std::is_base_of_v<ast::node, D>;
}