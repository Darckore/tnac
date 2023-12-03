//
// Compiler
//

#pragma once
#include "cfg/cfg.hpp"

namespace tnac
{
  class sema;
  class feedback;

  namespace ast
  {
    class node;
  }
}

namespace tnac
{
  //
  // Walks the AST and produces a control flow graph
  //
  class compiler final
  {
  public:
    using tree     = ast::node;
    using tree_ptr = tree*;
    using tree_ref = tree&;

  public:
    CLASS_SPECIALS_NONE(compiler);

    ~compiler() noexcept;

    compiler(sema& sema, feedback* fb) noexcept;

  public:
    //
    // Accepts an AST node and builds the corresponding CFG
    //
    void operator()(tree_ref node) noexcept;

  public:
    //
    // Returns a reference to the CFG
    // 
    // const version
    //
    const ir::cfg& cfg() const noexcept;

    //
    // Returns a reference to the CFG
    //
    ir::cfg& cfg() noexcept;

  private:
    sema* m_sema{};
    feedback* m_feedback{};
    ir::cfg m_cfg;
  };
}