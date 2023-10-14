//
// Cfg function
//

#pragma once
#include "compiler/cfg/basic_block.hpp"

namespace tnac::comp
{
  //
  // Represents a function in the cfg
  //
  class func final
  {
  public:
    using name_t = buf_t;

  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(func);

    ~func() noexcept;

    func(name_t name, func* parent) noexcept;

  public:
    //
    // Returns the function's mangled name
    //
    string_t name() const noexcept;

    //
    // Returns the function's parent (can be null)
    // 
    // const version
    //
    const func* parent() const noexcept;

    //
    // Returns the function's parent (can be null)
    //
    func* parent() noexcept;

  private:
    name_t m_name;
    func* m_parent{};
  };
}