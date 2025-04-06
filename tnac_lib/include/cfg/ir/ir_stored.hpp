//
// IR stored constants
//

#pragma once
#include "cfg/ir/ir_base.hpp"
#include "eval/value.hpp"

namespace tnac::ir
{
  class vreg;
}

namespace tnac::ir
{
  //
  // A representation of a constant interned in the static storage
  //
  class constant final :
    public utils::ilist_node<constant>,
    public node
  {
  public:
    using value_type = eval::value;

  public:
    CLASS_SPECIALS_NONE(constant);

    virtual ~constant() noexcept;

    constant(vreg& reg, value_type val) noexcept;

  public:
    //
    // Returns a reference to the global register
    // associated with the stored value
    //
    const vreg& target_reg() const noexcept;

    //
    // Returns a reference to the global register
    // associated with the stored value
    //
    vreg& target_reg() noexcept;

    //
    // Returns the interned value
    //
    const value_type& value() const noexcept;

  private:
    vreg* m_reg{};
    value_type m_value;
  };
}