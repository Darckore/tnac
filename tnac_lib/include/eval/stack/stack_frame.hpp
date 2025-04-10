//
// Eval stack frame
//

#pragma once
#include "eval/value/value.hpp"

namespace tnac::eval
{
  //
  // Holds execution state for a function
  //
  class stack_frame final
  {
  public:
    using param_count = std::uint16_t;
    using input_args  = std::vector<value>;
    using name_type   = string_t;

    using value_reg = std::unordered_map<entity_id, value>;

  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(stack_frame);

    ~stack_frame() noexcept;

    stack_frame(name_type fname, param_count argSz) noexcept;

  public:

  private:
    input_args m_args;
    value_reg m_values;
    name_type m_name;
  };
}