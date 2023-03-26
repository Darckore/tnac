//
// AST visitor
//

#pragma once
#include "ast/ast_nodes.hpp"

namespace tnac::ast
{
  namespace detail
  {
    template <typename Node, typename Visitor>
    concept visitable_node = requires(Visitor v, Node* n)
    {
      v.visit(n);
    };

    template <typename N>
    concept visitable_root =
      ast_node<N> &&
      std::is_same_v<std::remove_const_t<N>, node>;

    template <typename From, typename To> struct ast_caster {};

    template <ast_node From, ast_node To> requires (std::is_const_v<From>)
    struct ast_caster<From, To>
    {
      using type = const To*;
    };

    template <ast_node From, ast_node To> requires (!std::is_const_v<From>)
    struct ast_caster<From, To>
    {
      using type = To*;
    };

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

    template <typename T>
    using dest = detail::ast_caster<node_t, T>::type;

    static constexpr auto order = VO;

  private:
    static consteval auto is_top_down() noexcept
    {
      return order == detail::visit_order::TopDown;
    }
    static consteval auto is_bottom_up() noexcept
    {
      return order == detail::visit_order::BottomUp;
    }

  public:
    CLASS_SPECIALS_ALL(visitor);
    
    void operator()(node_ptr root) noexcept
    {
      visit_root(root);
    }

  private:
    void visit(detail::visitable_node<derived_t> auto* cur) noexcept
    {
      auto&& self = static_cast<derived_t&>(*this);
      self.visit(cur);
    }

    template <typename Node>
    void visit(Node*)
    {
    }

    void visit_impl(dest<error_expr> err) noexcept
    {
      visit(err);
    }

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

    void visit_impl(dest<binary_expr> binary) noexcept
    {
      if constexpr (is_top_down())
        visit(binary);

      visit_root(&binary->left());
      visit_root(&binary->right());

      if constexpr (is_bottom_up())
        visit(binary);
    }

    void visit_impl(dest<unary_expr> unary) noexcept
    {
      if constexpr (is_top_down())
        visit(unary);

      visit_root(&unary->operand());
      
      if constexpr (is_bottom_up())
        visit(unary);
    }

    void visit_impl(dest<paren_expr> paren) noexcept
    {
      if constexpr (is_top_down())
        visit(paren);
      
      visit_root(&paren->internal_expr());
      
      if constexpr (is_bottom_up())
        visit(paren);
    }

    void visit_impl(dest<id_expr> id) noexcept
    {
      visit(id);
    }

    void visit_impl(dest<lit_expr> lit) noexcept
    {
      visit(lit);
    }


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

      case Paren:
        visit_impl(cast<node_t, paren_expr>(cur));
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

  template <typename Derived>
  using top_down_visitor = detail::visitor<Derived, detail::visit_order::TopDown>;

  template <typename Derived>
  using const_top_down_visitor = detail::const_visitor<Derived, detail::visit_order::TopDown>;

  template <typename Derived>
  using bottom_up_visitor = detail::visitor<Derived, detail::visit_order::BottomUp>;

  template <typename Derived>
  using const_bottom_up_visitor = detail::const_visitor<Derived, detail::visit_order::BottomUp>;
}