//
// AST builder
//

#pragma once
#include "ast/ast_nodes.hpp"

namespace tnac::ast
{
  //
  // Creates and manages lifetime of AST nodes
  //
  class builder
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
      auto res = new (std::nothrow) T{ std::forward<Args>(args)... };
      m_store.emplace_back(res);
      return res;
    }

  public:
    //
    // Creates a scope
    //
    scope* make_scope(scope::elem_list children) noexcept;

    //
    // Creates a literal expression
    //
    expr* make_literal(const token& tok) noexcept;

    //
    // Creates a unary expression
    //
    unary_expr* make_unary(expr& e, const token& op) noexcept;

  private:
    owner_store m_store;
  };

}