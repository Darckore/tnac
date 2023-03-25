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

    template <typename From, typename To> struct cast_to {};

    template <ast_node From, ast_node To> requires (std::is_const_v<From>)
    struct cast_to<From, To>
    {
      using type = const To*;
    };

    template <ast_node From, ast_node To> requires (!std::is_const_v<From>)
    struct cast_to<From, To>
    {
      using type = To*;
    };
  }

  //
  // Base CRTP visitor for AST
  //
  template <typename Derived, detail::visitable_root N = node>
  class visitor
  {
  public:
    using derived_t = Derived;
    using node_t    = N;
    using node_ptr  = node_t*;
    using node_ref  = node_t&;

    template <ast_node Node>
    using cast_to = detail::cast_to<node_t, Node>::type;

  public:
    CLASS_SPECIALS_NONE(visitor);

    explicit visitor(node_ptr root) noexcept
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

    void visit_root(node_ptr cur) noexcept
    {
      if (!cur)
        return;

      using enum node_kind;

      switch (cur->what())
      {
      case Scope:
      {
        auto s = static_cast<cast_to<scope>>(cur);
        visit(s);
        for (auto child : s->children())
        {
          visit_root(child);
        }
      }
        break;
      case Literal:
      {
        auto lit = static_cast<cast_to<lit_expr>>(cur);
        visit(lit);
      }
        break;
      case Identifier:
      {
        auto id = static_cast<cast_to<id_expr>>(cur);
        visit(id);
      }
        break;
      case Unary:
      {
        auto unary = static_cast<cast_to<unary_expr>>(cur);
        visit(unary);
        visit_root(&unary->operand());
      }
        break;
      case Binary:
      {
        auto binary = static_cast<cast_to<binary_expr>>(cur);
        visit(binary);
        visit_root(&binary->left());
        visit_root(&binary->right());
      }
        break;
      case Paren:
      {
        auto paren = static_cast<cast_to<paren_expr>>(cur);
        visit(paren);
        visit_root(&paren->internal_expr());
      }
        break;
      case Error:
      {
        auto err = static_cast<cast_to<error_expr>>(cur);
        visit(err);
      }
        break;

      default:
        break;
      }
    }
  };
}