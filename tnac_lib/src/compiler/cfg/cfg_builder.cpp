#include "compiler/cfg/cfg_builder.hpp"
#include "compiler/cfg/cfg.hpp"

namespace tnac::comp
{
  // Special members

  cfg_builder::~cfg_builder() noexcept = default;

  cfg_builder::cfg_builder(cfg& gr) noexcept :
    m_cfg{ &gr }
  {
  }

  void cfg_builder::operator()(ast::node* root) noexcept
  {
    base::operator()(root);
  }


  // Static members

  cfg_builder::name_idx cfg_builder::gen_name_idx() noexcept
  {
    static name_idx idx{};
    return idx++;
  }

  cfg_builder::block_name cfg_builder::make_name(ast::scope& scope) noexcept
  {
    block_name res;

    if (scope.is_global())
      res = "`";

    return res;
  }


  // Public members

  cfg& cfg_builder::get() noexcept
  {
    return *m_cfg;
  }


  // Public members(Expressions)

  void cfg_builder::visit(ast::lit_expr& lit) noexcept
  {
    utils::unused(lit);
  }

  void cfg_builder::visit(ast::unary_expr& unary) noexcept
  {
    utils::unused(unary);
  }

  void cfg_builder::visit(ast::binary_expr& binary) noexcept
  {
    utils::unused(binary);
  }


  // Public members(Previews)

  bool cfg_builder::preview(ast::scope& scope) noexcept
  {
    auto&& block = get().create(make_name(scope));
    get().enter_block(block);
    return true;
  }

}