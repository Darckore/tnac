#include "evaluator/value_registry.hpp"

namespace tnac::eval
{
  // Special members

  registry::registry() noexcept = default;
  registry::~registry() noexcept = default;


  // Public members

  registry::value_type registry::register_int(int_type val) noexcept
  {
    auto&& v = register_val(val, m_ints);
    return { &v, type_id::Int };
  }
}