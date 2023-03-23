//
// Base AST
//

#pragma once

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
    friend class bulder;

  public:
    CLASS_SPECIALS_NONE(scope);

    virtual ~scope() noexcept;

  protected:
    explicit scope(node* parent) noexcept;

  private:
    elem_list m_children;
  };
}