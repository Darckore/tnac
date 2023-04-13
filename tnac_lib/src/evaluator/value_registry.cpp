#include "evaluator/value_registry.hpp"

namespace tnac::eval
{
  // Special members

  registry::registry() noexcept = default;
  registry::~registry() noexcept = default;


  // Public members

  registry::value_type registry::register_literal(int_type val) noexcept
  {
    return { &register_val(val, m_ints), type_id::Int };
  }

  registry::value_type registry::register_literal(float_type val) noexcept
  {
    return { &register_val(val, m_floats), type_id::Float };
  }

  registry::value_type registry::register_entity(entity_id id, int_type val) noexcept
  {
    return { &register_val(id, val), type_id::Int };
  }

  registry::value_type registry::register_entity(entity_id id, float_type val) noexcept
  {
    return { &register_val(id, val), type_id::Float };
  }

  registry::value_type registry::reset_result() noexcept
  {
    m_result = {};
    return evaluation_result();
  }

  registry::value_type registry::evaluation_result() const noexcept
  {
    return m_result;
  }

}