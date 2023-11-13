//
// Symbol
//

#pragma once

namespace tnac::semantics
{
  //
  // Represents a scope
  // We it need to keep track of where symbols are declared to disambiguate
  // between entities having the same name
  //
  class scope
  {
  public: // not for long
    const scope* m_enclosing{};
  };
}