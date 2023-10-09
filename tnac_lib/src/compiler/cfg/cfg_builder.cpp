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
      res = "global";

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
    auto&& tok = lit.pos();
    switch (tok.what())
    {
    case token::KwTrue:  get().consume_true();               break;
    case token::KwFalse: get().consume_false();              break;
    case token::KwI:     get().consume_i();                  break;
    case token::KwPi:    get().consume_pi();                 break;
    case token::KwE:     get().consume_e();                  break;
    case token::IntDec:  get().consume_int(tok.value(), 10); break;
    case token::IntBin:  get().consume_int(tok.value(), 2);  break;
    case token::IntOct:  get().consume_int(tok.value(), 8);  break;
    case token::IntHex:  get().consume_int(tok.value(), 16); break;
    case token::Float:   get().consume_float(tok.value());   break;

    default: break;
    }
  }

  void cfg_builder::visit(ast::unary_expr& unary) noexcept
  {
    utils::unused(unary);
  }

  void cfg_builder::visit(ast::binary_expr& binary) noexcept
  {
    utils::unused(binary);
  }


  // Public members(Scopes)

  bool cfg_builder::preview(ast::scope& scope) noexcept
  {
    auto&& block = get().create(make_name(scope));
    get().enter_block(block);
    return true;
  }

  void cfg_builder::visit(ast::scope& ) noexcept
  {
    get().exit_block();
  }

}