//
// Nodes for utility expressions
//

#pragma once
#include "parser/ast/ast_expr.hpp"

namespace tnac
{
  class parser;
}

namespace tnac::ast
{
  //
  // Error expression
  // Built instead of other expressions in cases where syntax errors arise
  //
  class error_expr final : public expr
  {
  private:
    friend class builder;

  public:
    CLASS_SPECIALS_NONE(error_expr);

    virtual ~error_expr() noexcept;

  protected:
    error_expr(const token& tok, string_t msg) noexcept;

  public:
    //
    // Returns the token resulted in a syntax error
    //
    const token& at() const noexcept;

    //
    // Returns the syntax error message
    //
    string_t message() const noexcept;

  private:
    string_t m_errMsg;
  };


  //
  // Command
  // Commands aren't actually included in the ast, or even returned as such
  // They are instructions to the driver to perform certain tasks
  // Declared here only because commands are produced in the parser, so they are
  // logically sort of related to the ast
  //
  class command final
  {
  private:
    friend class tnac::parser;

  public:
    using arg_list = std::vector<token>;
    using iterator = arg_list::const_iterator;
    using size_type = arg_list::size_type;

    using arg_baseline = std::span<const tok_kind>;

  public:
    CLASS_SPECIALS_NODEFAULT(command);

    ~command() noexcept;

  protected:
    command(const token& cmd, arg_list args) noexcept;

  public:
    //
    // Returns the argeter by the given index
    // Bounds checking is on the caller
    //
    const token& operator[](size_type idx) const noexcept;

    //
    // Returns the command's position token
    //
    const token& pos() const noexcept;

    //
    // Returns the command name
    //
    string_t name() const noexcept;

    //
    // Returns the number of argeters
    //
    size_type arg_count() const noexcept;

    //
    // Begin iterator to the arg list
    //
    iterator begin() const noexcept;

    //
    // End iterator to the arg list
    //
    iterator end() const noexcept;

  private:
    token m_cmd;
    arg_list m_args;
  };
}