#include "evaluator/value_registry.hpp"

namespace tnac::eval
{
  // Special members

  registry::registry() noexcept = default;
  registry::~registry() noexcept = default;


  // Public members

  registry::value_type registry::register_int(int_type val) noexcept
  {
    return { &register_val(val, m_ints), type_id::Int };
  }

  registry::value_type registry::register_float(float_type val) noexcept
  {
    return { &register_val(val, m_floats), type_id::Float };
  }
}