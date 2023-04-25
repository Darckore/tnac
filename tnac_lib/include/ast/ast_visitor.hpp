//
// AST visitor
//

#pragma once
#include "ast/ast_nodes.hpp"

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
    // Defines a root node of ast which can be visited
    //
    template <typename N>
    concept visitable_root =
      ast_node<N> &&
      std::is_same_v<std::remove_const_t<N>, node>;

    //
    // Unspecialised version of the cast helper
    // Its job is to maintain const consistency when casting between ast node types
    //
    template <typename From, typename To> struct ast_caster;

    //
    // Cast helper for const nodes
    //
    template <ast_node From, ast_node To> requires (std::is_const_v<From>)
    struct ast_caster<From, To>
    {
      using type = const To*;
    };

    //
    // Cast helper for non-const nodes
    //
    template <ast_node From, ast_node To> requires (!std::is_const_v<From>)
    struct ast_caster<From, To>
    {
      using type = To*;
    };

    //
    // Casts the given node to the specified type and maintains constness
    //
    template <typename From, typename To>
    auto cast(From* src) noexcept
    {
      return static_cast<ast_caster<From, To>::type>(src);
    }

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
    using dest = detail::ast_caster<node_t, T>::type;

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
    // Calls the appropriate visit method of the derived visitor class
    // if it is defined
    //
    void visit(detail::visitable_node<derived_t> auto* cur) noexcept
    {
      auto&& self = static_cast<derived_t&>(*this);
      self.visit(*cur);
    }

    //
    // Dumps the visit call for nodes for which the derived visitor class
    // doesn't provide a suitable visit method
    //
    template <typename Node>
    void visit(Node*)
    {
    }

    //
    // Visits an error expression
    //
    void visit_impl(dest<error_expr> err) noexcept
    {
      visit(err);
    }

    //
    // Visits a scope
    //
    void visit_impl(dest<scope> s) noexcept
    {
      if constexpr (is_top_down())
        visit(s);

      for (auto child : s->children())
      {
        visit_root(child);
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

      visit_root(&declExpr->declarator());

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

      visit_root(&varDecl->definition());

      if constexpr (is_bottom_up())
        visit(varDecl);
    }

    //
    // Visits an assign expression
    //
    void visit_impl(dest<assign_expr> assign) noexcept
    {
      if constexpr (is_top_down())
        visit(assign);

      visit_root(&assign->left());
      visit_root(&assign->right());

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

      visit_root(&binary->left());
      visit_root(&binary->right());

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

      visit_root(&unary->operand());
      
      if constexpr (is_bottom_up())
        visit(unary);
    }

    //
    // Visits a parenthesis expression
    //
    void visit_impl(dest<paren_expr> paren) noexcept
    {
      if constexpr (is_top_down())
        visit(paren);
      
      visit_root(&paren->internal_expr());
      
      if constexpr (is_bottom_up())
        visit(paren);
    }

    //
    // Visits a typed expression
    //
    void visit_impl(dest<typed_expr> typed) noexcept
    {
      if constexpr (is_top_down())
        visit(typed);

      for (auto param : typed->params())
      {
        visit_root(param);
      }

      if constexpr (is_bottom_up())
        visit(typed);
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
    void visit_root(node_ptr cur) noexcept
    {
      using detail::cast;

      if (!cur)
        return;

      using enum node_kind;

      switch (cur->what())
      {
      case Scope:
        visit_impl(cast<node_t, scope>(cur));
        break;

      case Result:
        visit_impl(cast<node_t, result_expr>(cur));
        break;

      case Literal:
        visit_impl(cast<node_t, lit_expr>(cur));
        break;

      case Identifier:
        visit_impl(cast<node_t, id_expr>(cur));
        break;

      case Unary:
        visit_impl(cast<node_t, unary_expr>(cur));
        break;

      case Binary:
        visit_impl(cast<node_t, binary_expr>(cur));
        break;

      case Assign:
        visit_impl(cast<node_t, assign_expr>(cur));
        break;

      case Decl:
        visit_impl(cast<node_t, decl_expr>(cur));
        break;

      case VarDecl:
        visit_impl(cast<node_t, var_decl>(cur));
        break;

      case Paren:
        visit_impl(cast<node_t, paren_expr>(cur));
        break;

      case Typed:
        visit_impl(cast<node_t, typed_expr>(cur));
        break;

      case Error:
        visit_impl(cast<node_t, error_expr>(cur));
        break;

      default:
        break;
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