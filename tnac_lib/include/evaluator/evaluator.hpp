//
// Expression evaluator
//

#pragma once
#include "ast/ast_visitor.hpp"
#include "evaluator/value_visitor.hpp"
#include "evaluator/value_registry.hpp"
#include "evaluator/call_stack.hpp"

namespace tnac
{
  namespace eval::detail
  {
    template <typename F>
    concept err_handler = std::is_nothrow_invocable_r_v<void, F, const token&, string_t>;
  }

  //
  // Evaluator for expressions
  // Visits the provided ast node bottom-up (children first)
  //
  class evaluator final : public ast::bottom_up_visitor<evaluator>
  {
  public:
    using base          = ast::bottom_up_visitor<evaluator>;
    using err_handler_t = std::function<void(const token&, string_t)>;
    using arg_list_t    = ast::invocation::arg_list;
    using size_type     = arg_list_t::size_type;

  public:
    CLASS_SPECIALS_NONE(evaluator);

    explicit evaluator(eval::registry& registry, eval::call_stack& callStack) noexcept;

    void operator()(ast::node* root) noexcept;

    //
    // Attaches the error handler
    //
    template <eval::detail::err_handler F>
    void on_error(F&& f) noexcept
    {
      m_errHandler = std::forward<F>(f);
    }

  public: // expressions
    //
    // Visits an assignment expression
    //
    void visit(ast::assign_expr& assign) noexcept;

    //
    // Visits a binary expression
    //
    void visit(ast::binary_expr& binary) noexcept;

    //
    // Visits a unary expression
    //
    void visit(ast::unary_expr& unary) noexcept;

    //
    // Visits a parenthesised expression
    //
    void visit(ast::paren_expr& paren) noexcept;

    //
    // Visits a typed expression
    //
    void visit(ast::typed_expr& expr) noexcept;

    //
    // Visits a call expression
    //
    void visit(ast::call_expr& expr) noexcept;

    //
    // Visits a ret expression
    //
    void visit(ast::ret_expr& ret) noexcept;

    //
    // Visits a literal expression
    //
    void visit(ast::lit_expr& lit) noexcept;

    //
    // Visits a variable reference expression
    //
    void visit(ast::id_expr& id) noexcept;

    //
    // Visits a result expression
    //
    void visit(ast::result_expr& res) noexcept;

  public: // decls
    //
    // Visits a declaration expression
    //
    void visit(ast::decl_expr& expr) noexcept;

    //
    // Visits a variable declarator
    //
    void visit(ast::var_decl& decl) noexcept;

    //
    // Visits a function declarator
    //
    void visit(ast::func_decl& decl) noexcept;

  public: // previews
    //
    // Generic preview
    //
    template <ast::ast_node Node>
    bool preview(Node&) noexcept
    {
      return !return_path();
    }

    //
    // Previews a function declaration
    // Needed to avoid evaluating the body or params before the function
    // actually gets called
    //
    bool preview(ast::func_decl& decl) noexcept;

    //
    // Pre-evaluates function arguments
    //
    bool preview(ast::call_expr& call) noexcept;

    //
    // Previews a binary
    // If it is not a logical expression, moves on
    // Else, calculates operands in order to short-circuit
    //
    bool preview(ast::binary_expr& expr) noexcept;

  private:
    //
    // Produces an evaluation error
    //
    void on_error(const token& pos, string_t msg) noexcept;

    //
    // Evaluates a literal and returns its value
    //
    eval::value eval_token(const token& tok) noexcept;

    //
    // Evaluates an assign expression and variable declarations 
    //
    void eval_assign(semantics::symbol& sym, eval::value rhs) noexcept;

    //
    // Creates a value for a function
    //
    void make_function(semantics::function& sym) noexcept;

    //
    // Attempts to initiate a call
    //
    bool init_call(semantics::function& sym, ast::call_expr& expr) noexcept;

    //
    // Returns true if a return is active
    //
    bool return_path() const noexcept;

    //
    // Converts a value to bool
    //
    bool to_bool(eval::value val) const noexcept;

  private:
    eval::value_visitor m_visitor;
    eval::call_stack& m_callStack;
    err_handler_t m_errHandler{};
    bool m_return{};
  };
}