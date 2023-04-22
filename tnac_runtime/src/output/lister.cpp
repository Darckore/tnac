#include "output/lister.hpp"

namespace tnac_rt::out
{
  namespace detail
  {
    namespace
    {
      template <ast::ast_node To>
      decltype(auto) to(const ast::node& from) noexcept
      {
        return static_cast<const To&>(from);
      }
    }
  }

  // Special members

  lister::~lister() noexcept = default;

  lister::lister() noexcept = default;

  // Public members

  void lister::operator()(const ast::node* node, out_stream& os) noexcept
  {
    m_out = &os;
    print(node);
  }

  void lister::operator()(const ast::node* node) noexcept
  {
    operator()(node, out());
  }

  // Private members

  void lister::print(const ast::node* root) noexcept
  {
    using enum ast::node_kind;
    using detail::to;

    if (!root)
      return;

    switch (root->what())
    {
    case Error:
      print(to<ast::error_expr>(*root));
      break;

    case Scope:
      print(to<ast::scope>(*root));
      break;

    case Literal:
      print(to<ast::lit_expr>(*root));
      break;

    case Identifier:
      print(to<ast::id_expr>(*root));
      break;

    case Unary:
      print(to<ast::unary_expr>(*root));
      break;

    case Binary:
      print(to<ast::binary_expr>(*root));
      break;

    case Assign:
      print(to<ast::assign_expr>(*root));
      break;

    case Decl:
      print(to<ast::decl_expr>(*root));
      break;

    case Paren:
      print(to<ast::paren_expr>(*root));
      break;

    case Result:
      print(to<ast::result_expr>(*root));
      break;

    case VarDecl:
      print(to<ast::var_decl>(*root));
      break;

    default:
      UTILS_ASSERT(false);
      break;
    }
  }

  void lister::print(const ast::scope& scope) noexcept
  {
    auto&& children = scope.children();
    const auto size = children.size();
    auto idx = std::size_t{};

    for (auto child : children)
    {
      print(child);
      ++idx;

      if (idx != size)
        out() << ':';

      out() << '\n';
    }
  }

  void lister::print(const ast::assign_expr& expr) noexcept
  {
    print(detail::to<ast::binary_expr>(expr));
  }

  void lister::print(const ast::decl_expr& expr) noexcept
  {
    print(&expr.declarator());
  }

  void lister::print(const ast::binary_expr& expr) noexcept
  {
    print(&expr.left());
    print_token(expr.op(), true);
    print(&expr.right());
  }

  void lister::print(const ast::unary_expr& expr) noexcept
  {
    print_token(expr.op(), false);
    print(&expr.operand());
  }

  void lister::print(const ast::paren_expr& expr) noexcept
  {
    out() << '(';
    print(&expr.internal_expr());
    out() << ") ";
  }

  void lister::print(const ast::lit_expr& expr) noexcept
  {
    print_token(expr.pos(), true);
  }

  void lister::print(const ast::id_expr& expr) noexcept
  {
    print_token(expr.pos(), true);
  }

  void lister::print(const ast::result_expr& expr) noexcept
  {
    print_token(expr.pos(), true);
  }

  void lister::print(const ast::error_expr& expr) noexcept
  {
    out() << "Error '" << expr.message() << "' ";
  }

  void lister::print(const ast::var_decl& decl) noexcept
  {
    print_token(decl.pos(), true);
    out() << "= ";
    print(&decl.definition());
  }

  void lister::endl() noexcept
  {
    out() << '\n';
  }

  out_stream& lister::out() noexcept
  {
    return *m_out;
  }

  void lister::print_token(const tnac::token& tok, bool addSpace) noexcept
  {
    out() << tok;
    if (addSpace)
      out() << ' ';
  }

}