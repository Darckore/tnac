//
// Parser
//

#pragma once
#include "parser/lex.hpp"
#include "ast/ast_builder.hpp"
#include "ast/ast_nodes.hpp"

namespace tnac
{
  class sema;
}

namespace tnac
{
  namespace detail
  {
    class op_precedence
    {
    public:
      enum class prec : std::uint8_t
      {
        LogicalOr,
        LogicalAnd,
        Equality,
        Relational,
        BitOr,
        BitXor,
        BitAnd,
        Additive,
        Multiplicative,
        Power,
        Unary
      };
      using enum prec;

    private:
      inline static constexpr std::array precOrder{
        LogicalAnd,
        Equality,
        Relational,
        BitOr,
        BitXor,
        BitAnd,
        Additive,
        Multiplicative,
        Power,
        Unary
      };


    public:
      CLASS_SPECIALS_ALL(op_precedence);

      constexpr op_precedence(prec cur) noexcept :
        m_cur{ cur }
      {}

      constexpr bool operator==(const op_precedence&) const noexcept = default;

      constexpr prec next() noexcept
      {
        using idx_t = decltype(precOrder)::size_type;
        const auto idx = static_cast<idx_t>(m_cur);
        return precOrder[idx];
      }

      constexpr auto operator*() const noexcept
      {
        return m_cur;
      }

    private:
      prec m_cur{};
    };

    template <typename F>
    concept err_handler = std::is_nothrow_invocable_r_v<void, F, const ast::error_expr&>;

    template <typename F>
    concept cmd_handler = std::is_nothrow_invocable_r_v<void, F, ast::command>;
  }

  //
  // Checks whether the given expression ends with a ';'
  // This implies an expression end, so, no need to add ':'
  //
  bool has_implicit_separator(const ast::node& expr) noexcept;

  //
  // Parser for the input
  //
  class parser final
  {
  public:
    using value_type = ast::node;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    using root_type = ast::scope;
    using root_ptr = root_type*;
    using const_root_ptr = const root_type*;
    using expr_list = root_type::elem_list;
    using param_list = ast::func_decl::param_list;

    using prec = detail::op_precedence;

    using err_handler_t = std::function<void(const ast::error_expr&)>;

    using cmd_handler_t = std::function<void(ast::command)>;

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
    // Helper type to manage scopes RAII-way
    //
    class scope_guard
    {
    public:
      CLASS_SPECIALS_NONE(scope_guard);

      scope_guard(parser& p, root_ptr newScope) noexcept :
        m_parser{ p }
      {
        m_parser.new_scope(newScope);
      }

      ~scope_guard() noexcept
      {
        m_parser.end_scope();
      }

    private:
      parser& m_parser;
    };

  public:
    CLASS_SPECIALS_NONE(parser);

    ~parser() noexcept;

    parser(ast::builder& builder, sema& sema) noexcept;

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
    pointer operator()(string_t str) noexcept;

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
    // Attaches the error handler which gets called when the parser encounters a
    // syntax error and produces an error expression
    //
    template <detail::err_handler F>
    void on_error(F&& handler) noexcept
    {
      m_errHandler = std::forward<F>(handler);
    }

    //
    // Attaches the command handler which gets called when a command is encountered
    //
    template <detail::cmd_handler F>
    void on_command(F&& handler) noexcept
    {
      m_cmdHandler = std::forward<F>(handler);
    }

  private: // semantics
    //
    // Instructs sema to open a new scope
    //
    void new_scope(root_ptr node) noexcept;

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
    // Previews the next token from the lexer
    //
    const token& peek_next() noexcept;

    //
    // Consumes and returns the next token
    //
    token next_tok() noexcept;

    //
    // Skips to one of the specified token kinds
    //
    template <typename ...Kinds>
      requires (static_cast<bool>(sizeof ...(Kinds)) && is_all_v<tok_kind, Kinds...>)
    void skip_to(Kinds ...kinds) noexcept
    {
      for (;;)
      {
        auto next = peek_next();
        if (next.is_eol() || next.is_any(kinds...))
          break;

        next_tok();
      }
    }

    //
    // Skips to either the expression separator ':', or EOL
    // Used to produce errors
    //
    void to_expr_end() noexcept;

    //
    // Produces an invalid expression for error recovery
    //
    ast::expr* error_expr(token pos, string_t msg, bool skipRest = false) noexcept;

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

    err_handler_t m_errHandler{};
    cmd_handler_t m_cmdHandler{};

    tok_kind m_terminateAt{ tok_kind::Eol };
  };
}