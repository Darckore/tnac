#include "packages/pkg/compiler_pkg.hpp"

namespace tnac::packages
{
  // Special members

  compiler::~compiler() noexcept = default;

  compiler::compiler(size_type stackSize) noexcept :
    m_callStack{ stackSize }
  {}


  // Public members

  void compiler::operator()(ast::node* root) noexcept
  {
    utils::unused(root);
  }

  eval::value compiler::last_result() const noexcept
  {
    return m_registry.evaluation_result();
  }
}