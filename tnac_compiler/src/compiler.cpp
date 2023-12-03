#include "compiler/compiler.hpp"
#include "common/feedback.hpp"
#include "common/diag.hpp"
#include "sema/sema.hpp"
#include "cfg/builder.hpp"

namespace tnac
{
  // Special members

  compiler::~compiler() noexcept = default;

  compiler::compiler(sema& sema, feedback* fb) noexcept :
    m_sema{ &sema },
    m_feedback{ fb }
  {}


  // Public members

  void compiler::operator()(tree_ref node) noexcept
  {
    utils::unused(node);
  }

  const ir::cfg& compiler::cfg() const noexcept
  {
    return m_cfg;
  }
  ir::cfg& compiler::cfg() noexcept
  {
    return FROM_CONST(cfg);
  }
}