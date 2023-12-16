//
// Control flow graph
//

#pragma once
#include "cfg/ir/ir_builder.hpp"

namespace tnac::ir
{
  //
  // Control flow graph of the program
  // Provides access to the IR
  //
  class cfg final
  {
  public:
    using name_t    = function::name_t;
    using size_type = std::size_t;

  public:
    CLASS_SPECIALS_NONE(cfg);

    ~cfg() noexcept;

    explicit cfg(builder& bld) noexcept;

  public:
    //
    // Declares a new module
    //
    function& declare_module(entity_id id, name_t name, size_type paramCount) noexcept;

    //
    // Declares a new function
    //
    function& declare_function(entity_id id, function& owner, name_t name, size_type paramCount) noexcept;

    //
    // Returns the module corresponding to the given id
    //
    function* find_module(entity_id id) noexcept;

  private:
    //
    // Casts and returns a function's parameter count from size_type
    //
    static function::size_type conv_param_count(size_type paramCount) noexcept;

  private:
    builder* m_builder;
  };
}