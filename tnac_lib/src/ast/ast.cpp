#include "ast/ast_nodes.hpp"

namespace tnac::ast
{
  // Base node

  node::~node() noexcept = default;

  node::node(kind k) noexcept :
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

  node::kind node::what() const noexcept
  {
    return m_kind;
  }

  bool node::is(kind k) const noexcept
  {
    return what() == k;
  }

  void node::make_child_of(node* parent) noexcept
  {
    m_parent = parent;
  }

  void node::assume_ancestry(node* child) noexcept
  {
    child->make_child_of(this);
  }

  // Scope

  scope::~scope() noexcept = default;

  scope::scope(elem_list children) noexcept :
    node{ node::Scope }
  {
    adopt(std::move(children));
  }

  void scope::adopt(elem_list children) noexcept
  {
    for (auto child : children)
    {
      node::assume_ancestry(child);
    }

    m_children.insert(m_children.end(), children.begin(), children.end());
  }

  const scope::elem_list& scope::children() const noexcept
  {
    return m_children;
  }
  scope::elem_list& scope::children() noexcept
  {
    return utils::mutate(std::as_const(*this).children());
  }
}