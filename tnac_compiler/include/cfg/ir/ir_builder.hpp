//
// IR builder
//

#pragma once
#include "cfg/ir/ir.hpp"

namespace tnac::ir
{
  //
  // Creates and manages lifetime of IR nodes
  //
  class builder final
  {
  public:
    using func_store  = std::unordered_map<entity_id, function>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(builder);

    ~builder() noexcept;

    builder() noexcept;

  public:
    //
    // Creates a module
    //
    function& make_module(entity_id id, function::name_t name, function::size_type paramCount) noexcept;

    //
    // Finds a stored function by id
    //
    function* find_function(entity_id id) noexcept;

  private:
    func_store m_functions;
  };
}