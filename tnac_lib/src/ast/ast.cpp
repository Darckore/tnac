#include "ast/ast_nodes.hpp"

namespace tnac::ast
{
  // Base node

  node::~node() noexcept = default;

  node::node(node* parent, kind k) noexcept :
    m_parent{ parent },
    m_kind{ k }
  {}

  const node* node::parent() const noexcept
  {
    return m_parent;
  }
  node* node::parent() noexcept
  {
    return utils::mutate(std::as_const(*this).parent());
  }

  node* node::to_base() noexcept
  {
    return this;
  }

  void node::make_child_of(node* parent) noexcept
  {
    m_parent = parent;
  }

  // Scope

  scope::~scope() noexcept = default;

  scope::scope(node* parent, elem_list children) noexcept :
    node{ parent, node::Scope },
    m_children{ std::move(children) }
  {
    assume_ancestry();
  }

  void scope::assume_ancestry() noexcept
  {
    for (auto child : m_children)
    {
      child->to_base()->make_child_of(this);
    }
  }
}