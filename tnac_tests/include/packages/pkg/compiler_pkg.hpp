//
// A convenient way to init everything the compiler needs
//

#pragma once
#include "exec/value/value_registry.hpp"
#include "exec/call_stack.hpp"

namespace tnac::packages
{
  //
  // A wrapper for all things compiler
  //
  class compiler final
  {
  public:
    using size_type = tnac::eval::call_stack::size_type;

  public:
    CLASS_SPECIALS_NONE(compiler);

    ~compiler() noexcept;

    explicit compiler(size_type stackSize) noexcept;

  public:
    //
    // Compiles the given ast into bytecode
    //
    void operator()(ast::node* root) noexcept;

    //
    // Last evaluation result
    //
    eval::value last_result() const noexcept;

  private:
    tnac::eval::registry m_registry;
    tnac::eval::call_stack m_callStack;
  };
}