//
// AST builder
//

#pragma once
#include "ast/ast.hpp"

namespace tnac
{
  //
  // Creates and manages lifetime of AST nodes
  //
  class ast_builder
  {
  public:
    CLASS_SPECIALS_NONE_CUSTOM(ast_builder);

    ast_builder() = default;
    ~ast_builder() = default;

  private:

  };

}