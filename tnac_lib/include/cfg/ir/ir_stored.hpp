//
// IR stored constants
//

#pragma once
#include "cfg/ir/ir_base.hpp"
#include "eval/value/value.hpp"

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


namespace tnac::ir
{
  //
  // Represents a structure (aka record) descriptor
  //
  class record final :
    public utils::ilist_node<record>,
    public node
  {
  public:
    using elem_list = std::vector<vreg*>;
    using size_type = elem_list::size_type;

  public:
    CLASS_SPECIALS_NONE(record);

    virtual ~record() noexcept;

    explicit record(size_type size) noexcept;

  public:
    //
    // Appends an element
    //
    record& append(vreg& elem) noexcept;

    //
    // Returns a pointer to the element register at the given index
    // or nullptr if out of bounds
    //
    const vreg* get_element(size_type idx) const noexcept;

    //
    // Returns a pointer to the element register at the given index
    // or nullptr if out of bounds
    //
    vreg* get_element(size_type idx) noexcept;

    //
    // Returns the number of elements
    //
    size_type size() const noexcept;

  private:
    elem_list m_elems;
  };
}