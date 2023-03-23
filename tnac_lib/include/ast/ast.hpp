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

    friend class builder;

  public:
    CLASS_SPECIALS_NONE(node);

    virtual ~node() noexcept = default;

  protected:
    node(node* parent, kind k) noexcept :
      m_parent{ parent },
      m_kind{ k }
    {}

  public:
    const node* parent() const noexcept
    {
      return m_parent;
    }
    node* parent() noexcept
    {
      return utils::mutate(std::as_const(*this).parent());
    }

  private:
    node* m_parent{};
    kind m_kind{ Error };
  };
}