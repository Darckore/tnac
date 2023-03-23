//
// Parser
//

#pragma once
#include "parser/lex.hpp"
#include "ast/ast_builder.hpp"
#include "ast/ast.hpp"

namespace tnac
{
  //
  // Parser for the input
  //
  class parser
  {
  public:
    using value_type = ast::node;
    using pointer = value_type*;
    using const_pointer = const value_type*;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(parser);

    ~parser() noexcept = default;
    parser() = default;

  public:
    //
    // Parses the input string and produces an AST
    //
    pointer parse(string_t str) noexcept;

    //
    // Returns the root node of the entire AST, which is,
    // potentially, built over multiple parse calls
    // 
    // const version
    //
    const_pointer root() const noexcept;

    //
    // Returns the root node of the entire AST, which is,
    // potentially, built over multiple parse calls
    //
    pointer root() noexcept;

  private:
    lex m_lex;
    ast::builder m_builder;
    pointer m_root{};
  };
}