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
    using err_handler_t = std::function<void(const token&, string_t)>;
    using arg_list_t    = ast::typed_expr::arg_list;
    using size_type     = arg_list_t::size_type;

  public:
    CLASS_SPECIALS_NONE(evaluator);

    explicit evaluator(eval::registry& registry, eval::call_stack& callStack) noexcept;

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
    // Previews a function declaration
    // Needed to avoid evaluating the body or params before the function
    // actually gets called
    //
    bool preview(ast::func_decl& decl) noexcept;


  private:
    //
    // Returns a reference to the call stack
    //
    eval::call_stack& call_stack() noexcept;

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

  private:
    eval::value_visitor m_visitor;
    eval::call_stack* m_callStack{};
    err_handler_t m_errHandler{};
  };
}