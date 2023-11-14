#include "parser/ast/ast.hpp"

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
    return FROM_CONST(parent);
  }

  node::kind node::what() const noexcept
  {
    return m_kind;
  }

  bool node::is(kind k) const noexcept
  {
    return what() == k;
  }

  bool node::is_valid() const noexcept
  {
    return m_valid;
  }

  void node::make_invalid() noexcept
  {
    m_valid = false;
  }

  void node::make_invalid_if(node* child) noexcept
  {
    if (!is_valid())
      return;

    if (child && !child->is_valid())
      make_invalid();
  }

  void node::make_child_of(node* parent) noexcept
  {
    m_parent = parent;
  }

  void node::assume_ancestry(node* child) noexcept
  {
    make_invalid_if(child);
    if (!child)
      return;

    child->make_child_of(this);
  }


  // Scope

  scope::~scope() noexcept = default;

  scope::scope(elem_list children) noexcept :
    scope{ node::Scope }
  {
    adopt(std::move(children));
  }

  scope::scope(kind k) noexcept :
    node{ k }
  {}

  void scope::adopt(elem_list children) noexcept
  {
    for (auto child : children)
    {
      assume_ancestry(child);
    }

    m_children.insert(m_children.end(), children.begin(), children.end());
  }

  const scope::elem_list& scope::children() const noexcept
  {
    return m_children;
  }
  scope::elem_list& scope::children() noexcept
  {
    return FROM_CONST(children);
  }

  bool scope::is_global() const noexcept
  {
    return !parent();
  }
}