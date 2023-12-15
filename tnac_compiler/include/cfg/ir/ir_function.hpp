//
// IR function
//

#pragma once
#include "cfg/ir/ir_base.hpp"

namespace tnac::ir
{
  //
  // Represents IR functions and modules
  //
  class function final : public node
  {
  public:
    using name_t = string_t;

  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(function);

    virtual ~function() noexcept;

  protected:
    explicit function(name_t name) noexcept;

    function(name_t name, function& owner) noexcept;

    function(name_t name, function* owner) noexcept;

  public:
    //
    // Returns the function or module name
    //
    name_t name() const noexcept;

    //
    // Returns a pointer to the owner function
    // 
    // const version
    //
    const function* owner_func() const noexcept;

    //
    // Returns a pointer to the owner function
    //
    function* owner_func() noexcept;

  private:
    name_t m_name;
    function* m_owner{};
  };
}