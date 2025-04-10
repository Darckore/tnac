//
// Evaluator for the IR
//

#pragma once
#include "eval/stack/stack_frame.hpp"
#include "eval/value/value.hpp"
#include "cfg/cfg.hpp"

namespace tnac
{
  //
  // Runs evaluation over the CFG with the given input data
  //
  class ir_eval final
  {
  public:
    CLASS_SPECIALS_NONE(ir_eval);

    ~ir_eval() noexcept;

    explicit ir_eval(ir::cfg& cfg) noexcept;

  private:
    ir::cfg* m_cfg{};
    eval::value m_result{};
  };
}