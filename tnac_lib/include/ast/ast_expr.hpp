//
// Expression nodes
//

#pragma once
#include "ast/ast.hpp"

namespace tnac::ast
{
  //
  // Base expression class
  //
  class expr : public node
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(expr);

    virtual ~expr() noexcept;

  protected:
    expr(node* parent, kind k) noexcept;
  };
}