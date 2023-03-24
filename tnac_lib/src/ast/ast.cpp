#include "ast/ast.hpp"

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


  // Scope

  scope::~scope() noexcept = default;

  scope::scope(node* parent, elem_list children) noexcept :
    node{ parent, node::Scope },
    m_children{ std::move(children) }
  {}
}