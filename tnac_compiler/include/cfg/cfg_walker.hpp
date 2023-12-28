//
// CFG walker
//

#pragma once
#include "cfg/cfg.hpp"
#include "cfg/ir/ir.hpp"

namespace tnac::ir
{
  namespace detail
  {
    template <typename G>
    concept ir_graph = utils::same_noquals<G, cfg>;

    template <ir_graph G> struct cfg_node_descr;
    template <> struct cfg_node_descr<cfg> { using type = ir::node; };
    template <> struct cfg_node_descr<const cfg> { using type = const ir::node; };

    template <ir_graph G>
    using cfg_node = typename cfg_node_descr<G>::type;

    //
    // Defines an IR node visitable by the specified visitor
    //
    template <typename Node, typename Visitor>
    concept visitable_ir = requires(Visitor v, Node* n)
    {
      v.visit(*n);
    };
  }

  //
  // Base visitor for the CFG and IR
  // Walks the CFG and calls the appropriate derived class's
  // visit methods on nodes
  //
  template <typename Derived, detail::ir_graph G>
  class cfg_walker
  {
  public:
    using derived_t = Derived;
    using graph_t   = G;
    using graph_ptr = graph_t*;
    using graph_ref = graph_t&;
    using node_t    = detail::cfg_node<graph_t>;
    using node_ptr  = node_t*;
    using node_ref  = node_t&;

    //
    // Destination type for casts between various IR kinds
    //
    template <typename T>
    using dest = decltype(utils::try_cast<T>(node_ptr{}));

  public:
    CLASS_SPECIALS_ALL(cfg_walker);

    //
    // Recursively walks the given CFG and its children
    //
    void operator()(graph_ref gr) noexcept
    {
      for (auto mod : gr)
        visit_root(mod);
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
    // Calls the appropriate visit method of the derived visitor class
    // if it is defined
    //
    void visit(detail::visitable_ir<derived_t> auto* cur) noexcept
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
    // Visits a function
    //
    void visit_impl(dest<function> fn) noexcept
    {
      for (auto nested : fn->children())
        visit_root(nested);

      visit(fn);
    }

    //
    // Visits a basic block
    //
    void visit_impl(dest<basic_block> bb) noexcept
    {
      visit(bb);
    }

    //
    // Visits an instruction
    //
    void visit_impl(dest<instruction> instr) noexcept
    {
      visit(instr);
    }

    //
    // Dispatches visit calls according to the node type
    //
    void visit_root(node_ptr n) noexcept
    {
      using utils::cast;

      if (!n)
        return;

      using enum ir_kind;
      auto&& cur = *n;

      switch (cur.what())
      {
      case Function:    visit_impl(&cast<function>(cur));    break;
      case Block:       visit_impl(&cast<basic_block>(cur)); break;
      case Instruction: visit_impl(&cast<instruction>(cur)); break;
      }
    }
  };

  //
  // Base type for a non-const CFG walker
  //
  template <typename Derived>
  using walker = cfg_walker<Derived, cfg>;

  //
  // Base type for a const CFG walker
  //
  template <typename Derived>
  using const_walker = cfg_walker<Derived, const cfg>;
}