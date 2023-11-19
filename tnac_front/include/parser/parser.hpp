//
// Parser
//

#pragma once
#include "parser/lex.hpp"
#include "ast/ast_builder.hpp"
#include "ast/ast.hpp"
#include "sema/sym/symbols.hpp"

namespace tnac
{
  class sema;
  class feedback;
  
  namespace src
  {
    class file;
  }

  namespace detail
  {
    class op_precedence;
  }

  //
  // Checks whether the given expression ends with a ';'
  // This implies an expression end, so, no need to add ':'
  //
  bool has_implicit_separator(const ast::node& expr) noexcept;
}

namespace tnac
{
  //
  // Parser for the input
  //
  class parser final
  {
  public:
    using value_type    = ast::node;
    using pointer       = value_type*;
    using const_pointer = const value_type*;

    using module_type      = ast::module_def;
    using module_ptr       = module_type*;
    using const_module_ptr = const module_type*;

    using root_type      = ast::root;
    using root_ptr       = root_type*;
    using const_root_ptr = const root_type*;

    using expr_list      = ast::scope::elem_list;
    using param_list     = ast::func_decl::param_list;
    using import_name    = ast::import_dir::elem_list;
    using import_alias   = ast::import_dir::pointer;

    using loc_t    = src::location;
    using tok_opt = lex::token_opt;

    using prec = detail::op_precedence;

  private:
    //
    // Helper for the expr list parser
    // Defines whether the expression list should belong to a global
    // or a nested scope
    //
    enum class scope_level : std::uint8_t
    {
      Global,
      Nested
    };

    //
    // Helper that is used to instruct error_expr on the location it should be using
    //
    enum class err_pos : std::uint8_t
    {
      Current,
      Last
    };

    //
    // Helper type to manage scopes RAII-way
    //
    class scope_guard;

  public:
    CLASS_SPECIALS_NONE(parser);

    ~parser() noexcept;

    parser(ast::builder& builder, sema& sema, feedback* fb = nullptr) noexcept;

  private:
    parser(ast::builder& builder, sema& sema, feedback* fb, root_ptr root) noexcept;

  public:
    //
    // Parses the input string and produces an AST
    // Will return the last parsed expression in the list separated by ':'
    // 
    // The result is supposed to be used in immediate mode where expressions
    // are meant to be processed one by one
    // 
    // Call root to get the entire AST
    //
    pointer operator()(string_t str) noexcept;

    //
    // Parses the input string and provides a source location to the lexer
    //
    pointer operator()(string_t str, loc_t& srcLoc) noexcept;

    //
    // Parses the given source file
    //
    pointer operator()(src::file& input) noexcept;

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

    //
    // Creates a new instance of the parser, maintaining the current root node
    //
    parser branch() const noexcept;

  private: // semantics
    //
    // Instructs sema to open a new scope
    //
    void new_scope(semantics::scope_kind kind) noexcept;

    //
    // Instructs sema to return to the previous scope
    //
    void end_scope() noexcept;

  private: // commands
    //
    // Parses a command along with its arguments and calls the current command handler
    // If no command handler is set, skips to the end of input, or the
    // nearest expression separator (if one exists)
    //
    void command(bool consumeSeparator) noexcept;

    //
    // Parses command arguments until the end of input, or an expression separator
    // is encountered
    //
    ast::command::arg_list command_args(bool consumeSeparator) noexcept;

  private: // parsing
    //
    // Opens scope for the current module
    //
    void start_module(loc_t& loc) noexcept;

    //
    // Inits the ast root if it doesn't yet exist
    //
    void init_root() noexcept;

    //
    // Previews the next token from the lexer
    //
    const token& peek_next() noexcept;

    //
    // Consumes and returns the next token
    //
    token next_tok() noexcept;

    //
    // Returns the last saved token, or the provided dummy one
    //
    const token& last_tok(const token& dummy) noexcept;

    //
    // Produces an invalid expression for error recovery
    //
    ast::expr* error_expr(const token& pos, string_t msg, err_pos at) noexcept;

    //
    // Parses the program
    //
    pointer program(string_t input, loc_t& loc) noexcept;

    //
    // Parses module entry
    //
    void entry() noexcept;

    //
    // Parses imports
    //
    void import_seq() noexcept;

    //
    // Parses an import directive
    //
    ast::import_dir* import_dir() noexcept;

    //
    // Parses a compound name of an imported module
    //
    import_name imported_module_name() noexcept;

    //
    // Parses an imported module alias
    //
    import_alias module_alias(semantics::module_sym& src) noexcept;

    //
    // Parses a list of expressions
    //
    expr_list expression_list(scope_level scopeLvl) noexcept;

    //
    // Parses an expression
    //
    ast::expr* expr() noexcept;

    //
    // Parses a decl expr
    //
    ast::expr* decl_expr() noexcept;

    //
    // Parses a ret expr
    //
    ast::expr* ret_expr() noexcept;

    //
    // Parses a declarator
    //
    ast::decl* declarator() noexcept;

    //
    // Parses a variable declarator
    //
    ast::decl* var_decl(token name) noexcept;

    //
    // Parses a function declarator
    //
    ast::decl* func_decl(token name) noexcept;

    //
    // Parses a parameter declarator
    //
    ast::param_decl* param_decl() noexcept;

    //
    // Parses function parameters
    //
    param_list formal_params() noexcept;

    //
    // Parses an assign expr
    //
    ast::expr* assign_expr() noexcept;

    //
    // Entry point into a binary expression parser
    //
    ast::expr* binary_expr() noexcept;

    //
    // Parses a binary expression according to operation precedence
    //
    ast::expr* binary_expr(prec precedence) noexcept;

    //
    // Dispatches parse calls according to operation precedence
    //
    ast::expr* expr_by_prec(prec precedence) noexcept;

    //
    // Parses a unary expr
    //
    ast::expr* unary_expr() noexcept;

    //
    // Parses an array expr
    //
    ast::expr* array_expr() noexcept;

    //
    // Parses a paren expr
    //
    ast::expr* paren_expr() noexcept;

    //
    // Parses an abs expr
    //
    ast::expr* abs_expr() noexcept;

    //
    // Parses an anonimous function expression
    //
    ast::expr* anonimous_function() noexcept;

    //
    // Parses a primary expr
    //
    ast::expr* primary_expr() noexcept;

    //
    // Parses a typed expr
    //
    ast::expr* typed_expr() noexcept;

    //
    // Parses an argument list
    //
    expr_list arg_list(tok_kind closing) noexcept;

    //
    // Parses a call expression
    // If the call is not present, produces an id expression
    //
    ast::expr* call_expr() noexcept;

    //
    // Parses a dot expression
    //
    ast::expr* dot_expr() noexcept;

    //
    // Parses a conditional expression
    //
    ast::expr* cond_expr() noexcept;

    //
    // Parses conditional expression's condition
    //
    ast::expr* cond() noexcept;

    //
    // Parses conditional expression's shorthand version
    //
    ast::expr* cond_short(ast::expr& condExpr, ast::scope& scope) noexcept;

    //
    // Parses conditional expression's body
    //
    ast::expr* cond_body(ast::expr& condExpr, ast::scope& scope) noexcept;

    //
    // Parses a pattern used in conditional expressions
    //
    ast::expr* cond_pattern() noexcept;

    //
    // Parses a pattern matcher
    //
    ast::expr* cond_matcher() noexcept;

  private:
    lex m_lex;
    ast::builder& m_builder;
    sema& m_sema;
    root_ptr m_root{};
    module_ptr m_curModule{};
    tok_opt m_lastConsumed{};

    feedback* m_feedback{};

    tok_kind m_terminateAt{ tok_kind::Eol };
  };
}