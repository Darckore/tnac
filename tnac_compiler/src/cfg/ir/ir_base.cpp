#include "cfg/ir/ir_base.hpp"

namespace tnac::ir
{
  // Special members

  node::~node() noexcept = default;

  node::node(kind k) noexcept :
    m_kind{ k }
  {}


  // Public members

  node::kind node::what() const noexcept
  {
    return m_kind;
  }
}