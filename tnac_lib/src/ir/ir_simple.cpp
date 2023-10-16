#include "ir/ir_simple.hpp"

namespace tnac::ir
{
  // Constant

  constant::~constant() noexcept = default;

  constant::constant(eval::value val) noexcept :
    wrapper{ op_code::Constant, val.size() + sizeof(eval::type_id) }
  {
    op().add_value(val);
  }
}