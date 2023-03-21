//
// Parser
//

#pragma once
#include "parser/lex.hpp"
#include "ast/ast.hpp"

namespace tnac
{
  //
  // Parser for the input
  //
  class parser
  {
  public:
    CLASS_SPECIALS_NONE_CUSTOM(parser);

    ~parser() noexcept = default;
    parser() = default;

  public:
    ast_node* parse(string_t str) noexcept;

  private:
    lex m_lex;
  };
}