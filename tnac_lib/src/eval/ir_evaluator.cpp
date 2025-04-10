#include "eval/ir_evaluator.hpp"

namespace tnac
{
  // Special members

  ir_eval::~ir_eval() noexcept = default;

  ir_eval::ir_eval(ir::cfg& cfg) noexcept :
    m_cfg{ &cfg }
  { }
}