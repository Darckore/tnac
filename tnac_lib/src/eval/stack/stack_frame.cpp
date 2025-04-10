#include "eval/stack/stack_frame.hpp"

namespace tnac::eval
{
  // Special members

  stack_frame::~stack_frame() noexcept = default;

  stack_frame::stack_frame(name_type fname, param_count argSz) noexcept :
    m_name{ fname }
  {
    m_args.reserve(argSz);
  }


  // Public members
}