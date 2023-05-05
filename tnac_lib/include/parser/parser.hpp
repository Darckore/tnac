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
        BitOr,
        BitXor,
        BitAnd,
        Additive,
        Multiplicative,
        Unary
      };
      using enum prec;

    private:
      inline static constexpr std::array precOrder{
        BitXor,
        BitAnd,
        Additive,
        Multiplicative,
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

    using prec = detail::op_precedence;

    using err_handler_t = std::function<void(const ast::error_expr&)>;

    using cmd_handler_t = std::function<void(ast::command)>;

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
    // Attaches the error handler which gets called when the parser encounteres a
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
    // Parses a command along with its parameters and calls the current command handler
    // If no command handler is set, skips to the end of input, or the
    // nearest expression separator (if one exists)
    //
    void command() noexcept;

    //
    // Parses command parameters until the end of input, or an expression separator
    // is encountered
    //
    ast::command::param_list command_params() noexcept;

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
    expr_list expression_list() noexcept;

    //
    // Parses an expression
    //
    ast::expr* expr() noexcept;

    //
    // Parses a decl expr
    //
    ast::expr* decl_expr() noexcept;

    //
    // Parses a declarator
    //
    ast::decl* declarator() noexcept;

    //
    // Parses a variable declarator
    //
    ast::decl* var_decl() noexcept;

    //
    // Parses an assign expr
    //
    ast::expr* assign_expr() noexcept;

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
    // Parses a paren expr
    //
    ast::expr* paren_expr() noexcept;

    //
    // Parses a primary expr
    //
    ast::expr* primary_expr() noexcept;

    //
    // Parses a typed expr
    //
    ast::expr* typed_expr() noexcept;

    //
    // Parses a parameter list
    //
    expr_list param_list() noexcept;

    //
    // Parses an id expression
    //
    ast::expr* id_expr() noexcept;

  private:
    lex m_lex;
    ast::builder& m_builder;
    sema& m_sema;
    root_ptr m_root{};

    err_handler_t m_errHandler{};
    cmd_handler_t m_cmdHandler{};
  };
}