//
// Compiler
//

#pragma once
#include "cfg/cfg.hpp"

namespace tnac
{
  class sema;
  class feedback;
}

namespace tnac
{
  //
  // Walks the AST and produces a control flow graph
  //
  class compiler final
  {
  public:
    CLASS_SPECIALS_NONE(compiler);

    ~compiler() noexcept;

    compiler(sema& sema, feedback* fb) noexcept;

  private:
    sema* m_sema{};
    feedback* m_feedback{};
    ir::cfg m_cfg;
  };
}