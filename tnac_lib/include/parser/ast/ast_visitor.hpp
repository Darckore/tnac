//
// AST visitor
//

#pragma once
#include "parser/ast/ast.hpp"

namespace tnac::ast
{
  namespace detail
  {
    //
    // Defines a derived visitor ability to react on exit from a scope child
    //
    template <typename N, typename Visitor>
    concept has_exit_child = 
      ast_node<N> &&
      requires(Visitor v, N* node)
    {
      { v.exit_child(*node) }->std::same_as<bool>;
    };

    //
    // Defines a derived visitor ability to react on exit from a scope child
    // in case when the derived class stopped further iteration by returning
    // false from it exit_child method
    // This provides access to the next child node which would be visited
    // if exit_child returned true
    //
    template <typename N, typename Visitor>
    concept has_post_exit =
      ast_node<N> &&
      requires(Visitor v, N* node)
    {
      v.post_exit(*node);
    };

    //
    // Defines a root node of ast which can be visited
    //
    template <typename N>
    concept visitable_root =
      ast_node<N> &&
      utils::same_noquals<N, node>;


    //
    // Constrols the order in which tree nodes are visited
    // - TopDown  first visits a node, then its children
    // - BottomUp first visits a node's children, then the node itself
    //
    enum class visit_order : std::uint8_t
    {
      TopDown,
      BottomUp
    };
  }

  //
  // Base CRTP visitor for AST
  //
  template <typename Derived, detail::visitable_root N, detail::visit_order VO>
  class visitor
  {
  public:
    using derived_t = Derived;
    using node_t    = N;
    using node_ptr  = node_t*;
    using node_ref  = node_t&;
    using loc_opt   = std::optional<src::loc_wrapper>;

  public:
    //
    // Destination type for casts between various ast kinds
    //
    template <typename T>
    using dest = std::conditional_t<std::is_const_v<node_t>, const T, T>*;

    static constexpr auto order = VO;

  private:
    //
    // Checks whether the target ast is visited top to bottom
    //
    static consteval auto is_top_down() noexcept
    {
      return order == detail::visit_order::TopDown;
    }

    //
    // Checks whether the target ast is visited bottom to top
    //
    static consteval auto is_bottom_up() noexcept
    {
      return order == detail::visit_order::BottomUp;
    }

  public:
    CLASS_SPECIALS_ALL(visitor);
    
    //
    // Recursively visits the given node and its children
    // according to the specified visit order
    //
    void operator()(node_ptr root) noexcept
    {
      visit_root(root);
    }

    //
    // Attempts to extract location from a node
    //
    loc_opt try_get_location(node_ref node) noexcept
    {
      return apply(&node, [this](auto n) noexcept
        {
          return get_loc(n);
        });
    }

  private:
    //
    // Casts itself to a reference to derived
    //
    derived_t& to_derived() noexcept
    {
      return static_cast<derived_t&>(*this);
    }

    //
    // Extracts location from an expression
    //
    loc_opt get_loc(dest<expr> e) noexcept
    {
      return e->pos().at();
    }

    //
    // Dumps extract location attempts on nodes that don't have it
    //
    loc_opt get_loc(node_ptr) noexcept
    {
      return {};
    }

    //
    // Is called after a child in a scope has been visited,
    // and another one is about to be entered
    //
    bool exit_child(detail::has_exit_child<derived_t> auto* child) noexcept
    {
      return to_derived().exit_child(*child);
    }

    //
    // Dumps the exit_child call for nodes for which the derived visitor class
    // doesn't provide a suitable method
    //
    template <typename Node>
    bool exit_child(Node*) noexcept
    {
      return true;
    }

    //
    // Called on the child node following the one for which
    // exit_child returned false
    //
    void post_exit(detail::has_post_exit<derived_t> auto* child) noexcept
    {
      to_derived().post_exit(*child);
    }

    //
    // Dumps the post_exit call for nodes for which the derived visitor class
    // doesn't provide a suitable method
    //
    template <typename Node>
    void post_exit(Node*) noexcept
    {
    }

    //
    // Calls the appropriate visit method of the derived visitor class
    // if it is defined
    //
    void visit(visitable<derived_t> auto* cur) noexcept
    {
      to_derived().visit(*cur);
    }

    //
    // Dumps the visit call for nodes for which the derived visitor class
    // doesn't provide a suitable visit method
    //
    template <typename Node>
    void visit(Node*) noexcept
    {
    }

    //
    // Previews the parent node to let the derived class decide whether to
    // visit its children or not
    // Mostly needed for bottom-up visitors
    //
    bool preview(previewable<derived_t> auto* cur) noexcept
    {
      return to_derived().preview(*cur);
    }

    //
    // If no custom preview logic is defined for this node type in the derived class,
    // instruct the base to visit its children unconditionally
    //
    template <typename Node>
    bool preview(Node*) noexcept
    {
      return true;
    }

    //
    // Stub visit_impl
    //
    void visit_impl(node_ptr) noexcept
    {
    }

    //
    // Visits a node list
    //
    void visit_children(auto&& list) noexcept
    {
      auto going = true;
      for (auto child : list)
      {
        if (!going)
        {
          post_exit(child);
          return;
        }

        visit_root(child);
        going = exit_child(child);
      }
    }

    //
    // Visits an error expression
    //
    void visit_impl(dest<error_expr> err) noexcept
    {
      visit(err);
    }

    //
    // Visits the AST root
    //
    void visit_impl(dest<root> r) noexcept
    {
      if constexpr (is_top_down())
        visit(r);

      if (preview(r))
      {
        visit_children(r->modules());
      }

      if constexpr (is_bottom_up())
        visit(r);
    }

    //
    // Visits a module definition
    //
    void visit_impl(dest<module_def> m) noexcept
    {
      if constexpr (is_top_down())
        visit(m);

      if (preview(m))
      {
        for (auto p : m->params())
          visit_root(p);

        for (auto i : m->imports())
          visit_root(i);

        visit_children(m->children());
      }

      if constexpr (is_bottom_up())
        visit(m);
    }

    //
    // Visits an import directive
    //
    void visit_impl(dest<import_dir> imp) noexcept
    {
      visit(imp);
    }

    //
    // Visits a scope
    //
    void visit_impl(dest<scope> s) noexcept
    {
      if constexpr (is_top_down())
        visit(s);

      if (preview(s))
      {
        visit_children(s->children());
      }

      if constexpr (is_bottom_up())
        visit(s);
    }

    //
    // Visits a declaration expression
    //
    void visit_impl(dest<decl_expr> declExpr) noexcept
    {
      if constexpr (is_top_down())
        visit(declExpr);

      if (preview(declExpr))
      {
        visit_root(&declExpr->declarator());
      }

      if constexpr (is_bottom_up())
        visit(declExpr);
    }

    //
    // Visits a variable declarator
    //
    void visit_impl(dest<var_decl> varDecl) noexcept
    {
      if constexpr (is_top_down())
        visit(varDecl);

      if (preview(varDecl))
      {
        visit_root(&varDecl->initialiser());
      }

      if constexpr (is_bottom_up())
        visit(varDecl);
    }

    //
    // Visits a parameter declarator
    //
    void visit_impl(dest<param_decl> paramDecl) noexcept
    {
      if constexpr (is_top_down())
        visit(paramDecl);

      if (preview(paramDecl))
      {
        visit_root(paramDecl->definition());
      }

      if constexpr (is_bottom_up())
        visit(paramDecl);
    }

    //
    // Visits a function declarator
    //
    void visit_impl(dest<func_decl> funcDecl) noexcept
    {
      if constexpr (is_top_down())
        visit(funcDecl);

      if (preview(funcDecl))
      {
        for (auto p : funcDecl->params())
          visit_root(p);

        visit_root(&funcDecl->body());
      }

      if constexpr (is_bottom_up())
        visit(funcDecl);
    }

    //
    // Visits an assign expression
    //
    void visit_impl(dest<assign_expr> assign) noexcept
    {
      if constexpr (is_top_down())
        visit(assign);

      if (preview(assign))
      {
        visit_root(&assign->left());
        visit_root(&assign->right());
      }

      if constexpr (is_bottom_up())
        visit(assign);
    }

    //
    // Visits a binary expression
    //
    void visit_impl(dest<binary_expr> binary) noexcept
    {
      if constexpr (is_top_down())
        visit(binary);

      if (preview(binary))
      {
        visit_root(&binary->left());
        visit_root(&binary->right());
      }

      if constexpr (is_bottom_up())
        visit(binary);
    }

    //
    // Visits a unary expression
    //
    void visit_impl(dest<unary_expr> unary) noexcept
    {
      if constexpr (is_top_down())
        visit(unary);

      if (preview(unary))
      {
        visit_root(&unary->operand());
      }
      
      if constexpr (is_bottom_up())
        visit(unary);
    }

    //
    // Visits a type check expression
    //
    void visit_impl(dest<type_check_expr> tcheck) noexcept
    {
      if constexpr (is_top_down())
        visit(tcheck);

      if (preview(tcheck))
      {
        visit_root(&tcheck->operand());
      }

      if constexpr (is_bottom_up())
        visit(tcheck);
    }

    //
    // Visits a type resolver expression
    //
    void visit_impl(dest<type_resolve_expr> tres) noexcept
    {
      if constexpr (is_top_down())
        visit(tres);

      if (preview(tres))
      {
        visit_root(&tres->checker());
        visit_root(&tres->resolver());
      }

      if constexpr (is_bottom_up())
        visit(tres);
    }

    //
    // Visits a tail expression
    //
    void visit_impl(dest<tail_expr> tail) noexcept
    {
      if constexpr (is_top_down())
        visit(tail);

      if (preview(tail))
      {
        visit_root(&tail->operand());
      }

      if constexpr (is_bottom_up())
        visit(tail);
    }

    //
    // Visits an array expr
    //
    void visit_impl(dest<array_expr> arr) noexcept
    {
      if constexpr (is_top_down())
        visit(arr);

      if (preview(arr))
      {
        for (auto elem : arr->elements())
        {
          visit_root(elem);
        }
      }

      if constexpr (is_bottom_up())
        visit(arr);
    }

    //
    // Visits a parenthesis expression
    //
    void visit_impl(dest<paren_expr> paren) noexcept
    {
      if constexpr (is_top_down())
        visit(paren);
      
      if (preview(paren))
      {
        visit_root(&paren->internal_expr());
      }
      
      if constexpr (is_bottom_up())
        visit(paren);
    }

    //
    // Visits an abs expression
    //
    void visit_impl(dest<abs_expr> abs) noexcept
    {
      if constexpr (is_top_down())
        visit(abs);

      if (preview(abs))
      {
        visit_root(&abs->expression());
      }

      if constexpr (is_bottom_up())
        visit(abs);
    }

    //
    // Visits a typed expression
    //
    void visit_impl(dest<typed_expr> typed) noexcept
    {
      if constexpr (is_top_down())
        visit(typed);

      if (preview(typed))
      {
        for (auto arg : typed->args())
        {
          visit_root(arg);
        }
      }

      if constexpr (is_bottom_up())
        visit(typed);
    }

    //
    // Visits a call expression
    //
    void visit_impl(dest<call_expr> call) noexcept
    {
      if constexpr (is_top_down())
        visit(call);

      if (preview(call))
      {
        visit_root(&call->callable());
        for (auto arg : call->args())
        {
          visit_root(arg);
        }
      }

      if constexpr (is_bottom_up())
        visit(call);
    }

    //
    // Visits a shorthand conditional
    //
    void visit_impl(dest<cond_short> cond) noexcept
    {
      if constexpr (is_top_down())
        visit(cond);

      if (preview(cond))
      {
        visit_root(&cond->cond());
        if (cond->has_true())  visit_root(&cond->on_true());
        if (cond->has_false()) visit_root(&cond->on_false());
      }

      if constexpr (is_bottom_up())
        visit(cond);
    }

    //
    // Visits a conditional expression
    //
    void visit_impl(dest<cond_expr> cond) noexcept
    {
      if constexpr (is_top_down())
        visit(cond);

      if (preview(cond))
      {
        visit_root(&cond->cond());
        visit_root(&cond->patterns());
      }

      if constexpr (is_bottom_up())
        visit(cond);
    }

    //
    // Visits a dot expression
    //
    void visit_impl(dest<dot_expr> dot) noexcept
    {
      if constexpr (is_top_down())
        visit(dot);

      if (preview(dot))
      {
        visit_root(&dot->accessed());
        visit_root(&dot->accessor());
      }

      if constexpr (is_bottom_up())
        visit(dot);
    }

    //
    // Visits a condition pattern
    //
    void visit_impl(dest<pattern> ptrn) noexcept
    {
      if constexpr (is_top_down())
        visit(ptrn);

      if (preview(ptrn))
      {
        visit_root(&ptrn->matcher());
        visit_root(&ptrn->body());
      }

      if constexpr (is_bottom_up())
        visit(ptrn);
    }

    //
    // Visits a pattern matcher
    //
    void visit_impl(dest<matcher> matchExpr) noexcept
    {
      if constexpr (is_top_down())
        visit(matchExpr);

      if (preview(matchExpr) && !matchExpr->is_default())
      {
        visit_root(&matchExpr->checked());
      }

      if constexpr (is_bottom_up())
        visit(matchExpr);
    }

    //
    // Visits a ret expression
    //
    void visit_impl(dest<ret_expr> ret) noexcept
    {
      if constexpr (is_top_down())
        visit(ret);

      if (preview(ret))
      {
        visit_root(&ret->returned_value());
      }

      if constexpr (is_bottom_up())
        visit(ret);
    }

    //
    // Visits a variable reference expression
    //
    void visit_impl(dest<id_expr> id) noexcept
    {
      visit(id);
    }

    //
    // Visits a literal expression
    //
    void visit_impl(dest<lit_expr> lit) noexcept
    {
      visit(lit);
    }

    //
    // Visits a result expression
    //
    void visit_impl(dest<result_expr> res) noexcept
    {
      visit(res);
    }

    //
    // Applies the given function according to the node type
    //
    template <typename F>
    auto apply(node_ptr n, F&& dispatch) noexcept
    {
      using utils::cast;
      using enum node_kind;
      auto&& cur = *n;

      switch (cur.what())
      {
      case Root:       return dispatch(&cast<root>(cur));
      case Module:     return dispatch(&cast<module_def>(cur));
      case Import:     return dispatch(&cast<import_dir>(cur));
      case Scope:      return dispatch(&cast<scope>(cur));
      case Result:     return dispatch(&cast<result_expr>(cur));
      case Ret:        return dispatch(&cast<ret_expr>(cur));
      case Literal:    return dispatch(&cast<lit_expr>(cur));
      case Identifier: return dispatch(&cast<id_expr>(cur));
      case Unary:      return dispatch(&cast<unary_expr>(cur));
      case IsType:     return dispatch(&cast<type_check_expr>(cur));
      case TypeRes:    return dispatch(&cast<type_resolve_expr>(cur));
      case Tail:       return dispatch(&cast<tail_expr>(cur));
      case Binary:     return dispatch(&cast<binary_expr>(cur));
      case Assign:     return dispatch(&cast<assign_expr>(cur));
      case Decl:       return dispatch(&cast<decl_expr>(cur));
      case VarDecl:    return dispatch(&cast<var_decl>(cur));
      case ParamDecl:  return dispatch(&cast<param_decl>(cur));
      case FuncDecl:   return dispatch(&cast<func_decl>(cur));
      case Array:      return dispatch(&cast<array_expr>(cur));
      case Paren:      return dispatch(&cast<paren_expr>(cur));
      case Abs:        return dispatch(&cast<abs_expr>(cur));
      case Typed:      return dispatch(&cast<typed_expr>(cur));
      case Call:       return dispatch(&cast<call_expr>(cur));
      case Matcher:    return dispatch(&cast<matcher>(cur));
      case Pattern:    return dispatch(&cast<pattern>(cur));
      case CondShort:  return dispatch(&cast<cond_short>(cur));
      case Cond:       return dispatch(&cast<cond_expr>(cur));
      case Dot:        return dispatch(&cast<dot_expr>(cur));
      case Error:      return dispatch(&cast<error_expr>(cur));

      default:         return dispatch(&cur);
      }
    }

    //
    // Dispatches visit calls according to the node type
    //
    void visit_root(node_ptr n) noexcept
    {
      if (!n) return;
      apply(n, [this](auto node) noexcept
        {
          visit_impl(node);
        });
    }
  };

  namespace detail
  {
    template <typename Derived, visit_order VO>
    using visitor = ast::visitor<Derived, ast::node, VO>;

    template <typename Derived, visit_order VO>
    using const_visitor = ast::visitor<Derived, const ast::node, VO>;
  }

  //
  // Base type for any top-down visitors of non-const ast nodes
  //
  template <typename Derived>
  using top_down_visitor = detail::visitor<Derived, detail::visit_order::TopDown>;

  //
  // Base type for any top-down visitors of const ast nodes
  //
  template <typename Derived>
  using const_top_down_visitor = detail::const_visitor<Derived, detail::visit_order::TopDown>;

  //
  // Base type for any bottom-up visitors of non-const ast nodes
  //
  template <typename Derived>
  using bottom_up_visitor = detail::visitor<Derived, detail::visit_order::BottomUp>;

  //
  // Base type for any bottom-up visitors of const ast nodes
  //
  template <typename Derived>
  using const_bottom_up_visitor = detail::const_visitor<Derived, detail::visit_order::BottomUp>;
}