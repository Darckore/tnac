//
// Base AST
//

#pragma once

namespace tnac
{
  //
  // Kinds of ast nodes
  //
  enum class ast_kind : std::uint16_t
  {
#include "general/ast_kinds.inl"
  };

  //
  // Base class for all AST nodes
  //
  class ast_node
  {
  public:
    using kind = ast_kind;
    using enum kind;

    friend class ast_builder;

  public:
    CLASS_SPECIALS_NONE(ast_node);

    virtual ~ast_node() noexcept = default;

  protected:
    ast_node(ast_node* parent, kind k) noexcept :
      m_parent{ parent },
      m_kind{ k }
    {}

  public:
    const ast_node* parent() const noexcept
    {
      return m_parent;
    }
    ast_node* parent() noexcept
    {
      return utils::mutate(std::as_const(*this).parent());
    }

  private:
    ast_node* m_parent{};
    kind m_kind{ Error };
  };
}