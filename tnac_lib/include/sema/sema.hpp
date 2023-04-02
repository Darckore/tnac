//
// Sema
//

#pragma once
#include "sema/sym_table.hpp"

namespace tnac
{
  //
  // Semantic analyser. Constrols scope tracking and is responsible
  // for registering and looking up symbols
  //
  class sema
  {
  public:
    CLASS_SPECIALS_NONE_CUSTOM(sema);

    ~sema() noexcept;
    sema() noexcept;

  public:

  private:
    semantics::sym_table m_symTab;
  };
}