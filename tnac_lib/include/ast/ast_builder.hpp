//
// AST builder
//

#pragma once
#include "ast/ast.hpp"

namespace tnac::ast
{
  //
  // Creates and manages lifetime of AST nodes
  //
  class builder
  {
  public:
    using base_node = node;
    using owning_ptr = std::unique_ptr<base_node>;
    using pointer = owning_ptr::pointer;
    using owner_store = std::vector<owning_ptr>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(builder);

    builder() noexcept;
    ~builder() noexcept;

  public:


  private:
    owner_store m_store;
  };

}