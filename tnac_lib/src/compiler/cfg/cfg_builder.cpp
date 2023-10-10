#include "compiler/cfg/cfg_builder.hpp"
#include "compiler/cfg/cfg.hpp"

namespace tnac::comp
{
  namespace detail
  {
    namespace
    {
      auto conv_unary(const token& tk) noexcept
      {
        using enum tok_kind;
        using enum eval::val_ops;
        switch (tk.what())
        {
        case Exclamation: return LogicalNot;
        case Question:    return LogicalIs;
        case Plus:        return UnaryPlus;
        case Minus:       return UnaryNegation;
        case Tilde:       return UnaryBitwiseNot;

        default: return InvalidOp;
        }
      }
      auto conv_binary(const token& tk) noexcept
      {
        using enum tok_kind;
        using enum eval::val_ops;
        switch (tk.what())
        {
        case Plus:     return Addition;
        case Minus:    return Subtraction;
        case Asterisk: return Multiplication;
        case Slash:    return Division;
        case Percent:  return Modulo;

        case Less:      return RelLess;
        case LessEq:    return RelLessEq;
        case Greater:   return RelGr;
        case GreaterEq: return RelGrEq;
        case Eq:        return Equal;
        case NotEq:     return NEqual;

        case Amp:  return BitwiseAnd;
        case Hat:  return BitwiseXor;
        case Pipe: return BitwiseOr;

        case Pow:  return BinaryPow;
        case Root: return BinaryRoot;

        default: return InvalidOp;
        }
      }
    }
  }

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
    get().consume_unary(detail::conv_unary(unary.op()));
  }

  void cfg_builder::visit(ast::binary_expr& binary) noexcept
  {
    get().consume_binary(detail::conv_binary(binary.op()));
  }


  // Public members(Scopes)

  bool cfg_builder::preview(ast::scope& scope) noexcept
  {
    utils::unused(scope);
    return true;
  }

  void cfg_builder::visit(ast::scope& ) noexcept
  {
    get().exit_block();
  }

}