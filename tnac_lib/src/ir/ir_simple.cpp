#include "ir/ir_simple.hpp"

namespace tnac::ir
{
  // Constant

  constant::~constant() noexcept = default;

  constant::constant(reg_index_t reg, eval::value val) noexcept :
    wrapper{ op_code::Constant, val.size() + sizeof(eval::type_id) + sizeof(reg) }
  {
    op().add_register(reg);
    op().add_value(val);
  }
}