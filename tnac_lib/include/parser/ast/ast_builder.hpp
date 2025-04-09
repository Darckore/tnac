//
// AST builder
//

#pragma once
#include "parser/ast/ast.hpp"

namespace tnac::ast
{
  //
  // Creates and manages lifetime of AST nodes
  //
  class builder final
  {
  public:
    using base_node = node;
    using owning_ptr = std::unique_ptr<base_node>;
    using pointer = owning_ptr::pointer;
    using owner_store = std::vector<owning_ptr>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(builder);

    builder() noexcept;
    ~builder() noexcept;

  private:
    //
    // Generic function to create a node of the given type
    // T must inherit from ast::node
    //
    template <ast_node T, typename ...Args>
    T* make(Args&& ...args) noexcept
    {
      auto res = new T{ std::forward<Args>(args)... };
      m_store.emplace_back(res);
      return res;
    }

  public: // General
    //
    // Creates a root node
    //
    root* make_root() noexcept;

    //
    // Creates a module definition
    //
    module_def* make_module(buf_t name, src::loc_wrapper loc) noexcept;

    //
    // Returns a default module
    //
    module_def* get_default_module(src::loc_wrapper loc) noexcept;

    //
    // Creates an import directive
    //
    import_dir* make_import(const token& pos, import_dir::elem_list name, import_dir::pointer alias) noexcept;

    //
    // Creates a scope
    //
    scope* make_scope(scope::elem_list children) noexcept;

    //
    // Creates an error expression
    //
    error_expr* make_error(const token& pos, string_t msg) noexcept;


  public: // Expressions
    //
    // Creates a result expression
    //
    result_expr* make_result(const token& tok) noexcept;

    //
    // Created a ret expression
    //
    ret_expr* make_ret(expr& retVal, const token& kwPos) noexcept;

    //
    // Creates a literal expression
    //
    lit_expr* make_literal(const token& tok) noexcept;

    //
    // Creates an id expression
    //
    id_expr* make_id(const token& tok, semantics::symbol& sym) noexcept;

    //
    // Creates a typed expression
    //
    typed_expr* make_typed(const token& kw, invocation::arg_list args) noexcept;

    //
    // Creates a call expression
    //
    call_expr* make_call(expr& callable, call_expr::arg_list args) noexcept;

    //
    // Creates an array expr
    //
    array_expr* make_array(const token& ob, array_expr::elem_list elements) noexcept;

    //
    // Creates a paren expression
    //
    paren_expr* make_paren(expr& e, const token& op) noexcept;

    //
    // Creates an abs expression
    //
    abs_expr* make_abs(expr& e, const token& op) noexcept;

    //
    // Creates a unary expression
    //
    unary_expr* make_unary(expr& e, const token& op) noexcept;

    //
    // Creates a tail expression
    //
    tail_expr* make_tail(expr& e) noexcept;

    //
    // Creates a binary expression
    //
    binary_expr* make_binary(expr& left, expr& right, const token& op) noexcept;

    //
    // Creates an assign expression
    //
    assign_expr* make_assign(expr& left, expr& right, const token& op) noexcept;

    //
    // Creates a matcher for a condition pattern
    //
    matcher* make_matcher(const token& op, expr* checkedExpr) noexcept;

    //
    // Creates a pattern used in conditionals
    //
    pattern* make_pattern(expr& matcherExpr, scope& body) noexcept;

    //
    // Creates a conditional expression
    //
    cond_expr* make_conditional(expr& condition, scope& body) noexcept;

    //
    // Creates a shorthand conditional
    //
    cond_short* make_short_cond(expr& condition, expr* onTrue, expr* onFalse, scope& sc) noexcept;

    //
    // Creates a dot expression
    //
    dot_expr* make_dot(expr& accessed, expr& accessor) noexcept;

  public: // Declarations
    //
    // Makes a decl expression
    //
    decl_expr* make_decl_expr(decl& d) noexcept;

    //
    // Makes a variable declarator
    //
    var_decl* make_var_decl(const token& var, expr& initialiser) noexcept;

    //
    // Makes a parameter declarator
    //
    param_decl* make_param_decl(const token& name, expr* opt) noexcept;

    //
    // Makes a function declarator
    //
    func_decl* make_func_decl(const token& func, const token& pos, scope& def, func_decl::param_list params) noexcept;

  private:
    owner_store m_store;
    module_def m_fakeModule;
  };

}