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
    using func_store  = std::list<function>;
    using instr_ptr   = std::unique_ptr<instruction>;
    using instr_store = std::list<instr_ptr>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(builder);

    ~builder() noexcept;

    builder() noexcept;

  private:
    func_store m_functions;
  };
}