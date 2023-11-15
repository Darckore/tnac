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
    // Defines an ast visitable by the specified visitor
    //
    template <typename Node, typename Visitor>
    concept visitable_node = requires(Visitor v, Node* n)
    {
      v.visit(*n);
    };

    //
    // Defines an ast previewable by the specified visitor
    //
    template <typename Node, typename Visitor>
    concept previewable_node = requires(Visitor v, Node * n)
    {
      { v.preview(*n) }->std::same_as<bool>;
    };

    //
    // Defines a derived visitor ability to react on exit from a scope child
    //
    template <typename Visitor>
    concept has_exit_child = requires(Visitor v)
    {
      { v.exit_child() }->std::same_as<bool>;
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

    //
    // Destination type for casts between various ast kinds
    //
    template <typename T>
    using dest = decltype(utils::try_cast<T>(node_ptr{}));

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

  private:
    //
    // Casts itself to a reference to derived
    //
    derived_t& to_derived() noexcept
    {
      return static_cast<derived_t&>(*this);
    }

    //
    // Is called after a child in a scope has been visited,
    // and another one is about to be entered
    //
    bool exit_child() noexcept
    {
      if constexpr (detail::has_exit_child<derived_t>)
      {
        return to_derived().exit_child();
      }
      else
      {
        return true;
      }
    }

    //
    // Calls the appropriate visit method of the derived visitor class
    // if it is defined
    //
    void visit(detail::visitable_node<derived_t> auto* cur) noexcept
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
    bool preview(detail::previewable_node<derived_t> auto* cur) noexcept
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
        for (auto child : r->modules())
        {
          visit_root(child);
          if (!exit_child())
            break;
        }
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
        for (auto child : m->children())
        {
          visit_root(child);
          if (!exit_child())
            break;
        }
      }

      if constexpr (is_bottom_up())
        visit(m);
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
        for (auto child : s->children())
        {
          visit_root(child);
          if (!exit_child())
            break;
        }
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
    // Dispatches visit calls according to the node type
    //
    void visit_root(node_ptr n) noexcept
    {
      using utils::cast;

      if (!n)
        return;

      using enum node_kind;
      auto&& cur = *n;

      switch (cur.what())
      {
      case Root:       visit_impl(&cast<root>(cur));  break;
      case Module:     visit_impl(&cast<module_def>(cur));  break;
      case Scope:      visit_impl(&cast<scope>(cur));       break;
      case Result:     visit_impl(&cast<result_expr>(cur)); break;
      case Ret:        visit_impl(&cast<ret_expr>(cur));    break;
      case Literal:    visit_impl(&cast<lit_expr>(cur));    break;
      case Identifier: visit_impl(&cast<id_expr>(cur));     break;
      case Unary:      visit_impl(&cast<unary_expr>(cur));  break;
      case Binary:     visit_impl(&cast<binary_expr>(cur)); break;
      case Assign:     visit_impl(&cast<assign_expr>(cur)); break;
      case Decl:       visit_impl(&cast<decl_expr>(cur));   break;
      case VarDecl:    visit_impl(&cast<var_decl>(cur));    break;
      case ParamDecl:  visit_impl(&cast<param_decl>(cur));  break;
      case FuncDecl:   visit_impl(&cast<func_decl>(cur));   break;
      case Array:      visit_impl(&cast<array_expr>(cur));  break;
      case Paren:      visit_impl(&cast<paren_expr>(cur));  break;
      case Abs:        visit_impl(&cast<abs_expr>(cur));    break;
      case Typed:      visit_impl(&cast<typed_expr>(cur));  break;
      case Call:       visit_impl(&cast<call_expr>(cur));   break;
      case Matcher:    visit_impl(&cast<matcher>(cur));     break;
      case Pattern:    visit_impl(&cast<pattern>(cur));     break;
      case CondShort:  visit_impl(&cast<cond_short>(cur));  break;
      case Cond:       visit_impl(&cast<cond_expr>(cur));   break;
      case Error:      visit_impl(&cast<error_expr>(cur));  break;
      }
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