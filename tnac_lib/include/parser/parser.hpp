//
// Parser
//

#pragma once
#include "parser/lex.hpp"
#include "ast/ast_builder.hpp"
#include "ast/ast_nodes.hpp"

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

    using root_type = ast::scope;
    using root_ptr = root_type*;
    using const_root_ptr = const root_type*;
    using expr_list = root_type::elem_list;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(parser);

    ~parser() noexcept = default;
    parser() = default;

  public:
    //
    // Parses the input string and produces an AST
    // Will return the last parsed expression in the list separated by ':'
    // 
    // The result is supposed to be used in immediate mode where expressions
    // are meant to be processed one by one
    // 
    // Call root to the get the entire AST
    //
    pointer parse(string_t str) noexcept;

    //
    // Returns the root node of the entire AST, which is,
    // potentially, built over multiple parse calls
    // 
    // const version
    //
    const_root_ptr root() const noexcept;

    //
    // Returns the root node of the entire AST, which is,
    // potentially, built over multiple parse calls
    //
    root_ptr root() noexcept;

  private:
    //
    // Previews the next token from the lexer
    //
    const token& peek_next() noexcept;

    //
    // Skips to either the expression separator ':', or EOL
    // Used to produce errors
    //
    void to_expr_end() noexcept;

    //
    // Produces an invalid expression for error recovery
    //
    ast::expr* error_expr(string_t msg) noexcept;

    //
    // Parses a list of expressions
    //
    expr_list expression_list() noexcept;

    //
    // Parses an expression
    //
    ast::expr* expr() noexcept;

    //
    // Parses an additive expr
    //
    ast::expr* additive_expr() noexcept;

    //
    // Parses a multiplicative expr
    //
    ast::expr* multiplicative_expr() noexcept;

    //
    // Parses a unary expr
    //
    ast::expr* unary_expr() noexcept;

    //
    // Parses a paren expr
    //
    ast::expr* paren_expr() noexcept;

    //
    // Parses a primary expr
    //
    ast::expr* primary_expr() noexcept;

  private:
    lex m_lex;
    ast::builder m_builder;
    root_ptr m_root{};
  };
}