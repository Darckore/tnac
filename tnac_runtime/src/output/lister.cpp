#include "output/lister.hpp"
#include "parser/parser.hpp"
#include "output/formatting.hpp"

namespace tnac::rt::out
{
  // Special members

  lister::~lister() noexcept = default;

  lister::lister() noexcept = default;

  // Public members

  void lister::operator()(const ast::node* node, out_stream& os) noexcept
  {
    m_out = &os;
    default_style();
    print(node);
    fmt::clear_clr(out());
  }

  void lister::operator()(const ast::node* node) noexcept
  {
    operator()(node, out());
  }


  // Private members

  void lister::print(const ast::node* root) noexcept
  {
    using enum ast::node_kind;
    using utils::cast;

    if (!root)
      return;

    indent(*root);
    switch (root->what())
    {
    case Error:      print(cast<ast::error_expr>(*root));      break;
    case Root:       print(cast<ast::root>(*root));            break;
    case Module:     print(cast<ast::module_def>(*root));      break;
    case Import:     print(cast<ast::import_dir>(*root));      break;
    case Scope:      print(cast<ast::scope>(*root));           break;
    case Literal:    print(cast<ast::lit_expr>(*root));        break;
    case Identifier: print(cast<ast::id_expr>(*root));         break;
    case Unary:      print(cast<ast::unary_expr>(*root));      break;
    case Tail:       print(cast<ast::tail_expr>(*root));       break;
    case IsType:     print(cast<ast::type_check_expr>(*root)); break;
    case Binary:     print(cast<ast::binary_expr>(*root));     break;
    case Assign:     print(cast<ast::assign_expr>(*root));     break;
    case Decl:       print(cast<ast::decl_expr>(*root));       break;
    case Abs:        print(cast<ast::abs_expr>(*root));        break;
    case Array:      print(cast<ast::array_expr>(*root));      break;
    case Paren:      print(cast<ast::paren_expr>(*root));      break;
    case Typed:      print(cast<ast::typed_expr>(*root));      break;
    case Call:       print(cast<ast::call_expr>(*root));       break;
    case Result:     print(cast<ast::result_expr>(*root));     break;
    case Ret:        print(cast<ast::ret_expr>(*root));        break;
    case CondShort:  print(cast<ast::cond_short>(*root));      break;
    case Cond:       print(cast<ast::cond_expr>(*root));       break;
    case Dot:        print(cast<ast::dot_expr>(*root));        break;
    case Pattern:    print(cast<ast::pattern>(*root));         break;
    case Matcher:    print(cast<ast::matcher>(*root));         break;
    case VarDecl:    print(cast<ast::var_decl>(*root));        break;
    case ParamDecl:  print(cast<ast::param_decl>(*root));      break;
    case FuncDecl:   print(cast<ast::func_decl>(*root));       break;

    default: UTILS_ASSERT(false); break;
    }
  }

  void lister::print(const ast::root& root) noexcept
  {
    for (auto mod : root.modules())
      print(mod);
  }

  void lister::print(const ast::module_def& mod) noexcept
  {
    auto name = mod.is_fake() ? "<fake>"sv : mod.name();
    comment_style();
    out() << "\\Module: " << name;
    default_style();
    endl();
    for (auto importDir : mod.imports())
    {
      print(importDir);
    }
    if (auto&& params = mod.params(); !params.empty())
    {
      kw_style();
      out() << "_entry ";
      default_style();
      print_params(params);
      endl();
    }
    print(utils::cast<ast::scope>(mod));
  }

  void lister::print(const ast::import_dir& id) noexcept
  {
    kw_style();
    out() << "_import ";
    default_style();
    auto&& name = id.name();
    if (!name.empty())
    {
      for (auto last = name.back(); auto part : name)
      {
        print_token(part->pos(), false);
        if (part != last)
          out() << '.';
      }
    }
    if (auto alias = id.alias_name())
    {
      kw_style();
      out() << " _as ";
      default_style();
      print_token(alias->pos(), false);
    }
    endl();
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
      if (child->is(ast::node_kind::Error))
      {
        endl();
        continue;
      }

      if (idx != size && !has_implicit_separator(*child))
        out() << ':';

      endl();
    }
  }

  void lister::print(const ast::assign_expr& expr) noexcept
  {
    print(utils::cast<ast::binary_expr>(expr));
  }

  void lister::print(const ast::decl_expr& expr) noexcept
  {
    print(&expr.declarator());
  }

  void lister::print(const ast::binary_expr& expr) noexcept
  {
    print(&expr.left());
    print_token(expr.op(), false);
    print(&expr.right());
  }

  void lister::print(const ast::unary_expr& expr) noexcept
  {
    print_token(expr.op(), false);
    print(&expr.operand());
  }

  void lister::print(const ast::type_check_expr& expr) noexcept
  {
    print_token(expr.type(), false);
    out() << "? ";
    print(&expr.operand());
  }

  void lister::print(const ast::tail_expr& expr) noexcept
  {
    print(&expr.operand());
    out() << '@';
  }

  void lister::print(const ast::array_expr& expr) noexcept
  {
    print_args(expr.elements(), '[', ']');
  }

  void lister::print(const ast::paren_expr& expr) noexcept
  {
    out() << '(';
    print(&expr.internal_expr());
    out() << ')';
  }

  void lister::print(const ast::abs_expr& expr) noexcept
  {
    out() << '|';
    print(&expr.expression());
    out() << '|';
  }

  void lister::print(const ast::typed_expr& expr) noexcept
  {
    print_invocation(expr);
  }

  void lister::print(const ast::call_expr& expr) noexcept
  {
    print(&expr.callable());
    print_args(expr.args(), '(', ')');
  }

  void lister::print(const ast::lit_expr& expr) noexcept
  {
    print_token(expr.pos(), false);
  }

  void lister::print(const ast::id_expr& expr) noexcept
  {
    print_token(expr.pos(), false);
  }

  void lister::print(const ast::ret_expr& expr) noexcept
  {
    print_token(expr.pos(), true);
    print(&expr.returned_value());
  }

  void lister::print(const ast::result_expr& expr) noexcept
  {
    print_token(expr.pos(), false);
  }

  void lister::print(const ast::error_expr& expr) noexcept
  {
    fmt::add_clr(out(), fmt::clr::Red);
    out() << "\\" << expr.message();
    default_style();
  }

  void lister::print(const ast::cond_short& expr) noexcept
  {
    out() << "{";
    print(&expr.cond());
    out() << '}';

    out() << "->{";
    
    if (expr.has_true())
    {
      print(&expr.on_true());
    }
    if (expr.has_false())
    {
      out() << ", ";
      print(&expr.on_false());
    }
    
    out() << "}";
  }

  void lister::print(const ast::cond_expr& expr) noexcept
  {
    out() << "{";
    print(&expr.cond());
    out() << '}';

    if (auto&& patterns = expr.patterns(); !patterns.children().empty())
    {
      endl();
      VALUE_GUARD(m_indent);
      ++m_indent;
      print(patterns);
      --m_indent;
      indent(nearest_to_scope(expr));
    }

    out() << "; ";
  }

  void lister::print(const ast::dot_expr& expr) noexcept
  {
    print(&expr.accessed());
    out() << '.';
    print(&expr.accessor());
  }

  void lister::print(const ast::pattern& expr) noexcept
  {
    print(&expr.matcher());

    endl();
    if (auto&& body = expr.body(); !body.children().empty())
    {
      VALUE_GUARD(m_indent);
      ++m_indent;
      print(body);
      indent(*expr.parent());
    }

    indent(expr);
    out() << ";";
  }

  void lister::print(const ast::matcher& expr) noexcept
  {
    out() << "{";
    if (!expr.is_default() && !expr.is_unary())
    {
      if (!expr.has_implicit_op())
        print_token(expr.pos(), false);

      print(&expr.checked());
    }
    if (expr.is_unary())
    {
      print_token(expr.pos(), false);
    }

    out() << "}->";
  }

  void lister::print(const ast::var_decl& decl) noexcept
  {
    print_token(decl.pos(), false);
    out() << "=";
    print(&decl.initialiser());
  }

  void lister::print(const ast::param_decl& decl) noexcept
  {
    id_style();
    out() << decl.name();
    default_style();
  }

  void lister::print(const ast::func_decl& decl) noexcept
  {
    auto&& pos = decl.pos();
    if (pos.is_identifier())
    {
      kw_style();
      out() << "_fn ";
      default_style();
    }

    print_token(decl.pos(), false);
    print_params(decl.params());

    if (auto&& body = decl.body(); !body.children().empty())
    {
      endl();
      VALUE_GUARD(m_indent);
      ++m_indent;
      print(body);
      --m_indent;
      indent(nearest_to_scope(decl));
    }
    else
    {
      out() << ' ';
    }

    out() << "; ";
  }

  const ast::node& lister::nearest_to_scope(const ast::node& src) noexcept
  {
    auto res = src.parent();
    for (;;)
    {
      auto next = res->parent();
      if (!next || next->is_any(ast::node_kind::Scope, ast::node_kind::Module))
        break;
      res = next;
    }
    return *res;
  }

  void lister::print_invocation(const ast::invocation& expr) noexcept
  {
    print_token(expr.name(), false);
    print_args(expr.args(), '(', ')');
  }

  void lister::print_args(const args_t& args, char_t open, char_t close) noexcept
  {
    out() << open;
    const auto size = args.size();
    auto idx = std::size_t{};
    for (auto arg : args)
    {
      print(arg);
      ++idx;

      if (idx != size)
        out() << ", ";
    }

    out() << close;
  }

  void lister::print_params(const params_t& params) noexcept
  {
    out() << '(';

    auto idx = std::size_t{};
    const auto size = params.size();
    for (auto param : params)
    {
      print(param);
      ++idx;

      if (idx != size)
        out() << ", ";
    }

    out() << ')';
  }

  void lister::indent(const ast::node& cur) noexcept
  {
    if (cur.is_any(ast::node_kind::Scope, ast::node_kind::Module))
      return;

    if (auto parent = cur.parent();
            !parent || !parent->is_any(ast::node_kind::Scope, ast::node_kind::Module))
    {
      return;
    }

    for (auto indentIdx = std::size_t{}; indentIdx < m_indent; ++indentIdx)
    {
      for (auto spcIdx = std::size_t{}; spcIdx < spacesPerIndent; ++spcIdx)
        out() << ' ';
    }
  }

  void lister::endl() noexcept
  {
    out() << '\n';
  }

  out_stream& lister::out() noexcept
  {
    return *m_out;
  }

  void lister::print_token_plain(const token& tok, bool addSpace) noexcept
  {
    out() << tok;
    if (addSpace)
      out() << ' ';
  }

  void lister::print_token(const token& tok, bool addSpace) noexcept
  {
    if (tok.is_keyword())
      kw_style();
    else if (tok.is_literal())
      lit_style();
    else if (tok.is_identifier())
      id_style();

    print_token_plain(tok, addSpace);
    default_style();
  }

  void lister::reset_style() noexcept
  {
    fmt::clear_clr(out());
    fmt::add_clr(out(), fmt::clr::White);
  }
  void lister::id_style() noexcept
  {
    reset_style();
    fmt::add_clr(out(), fmt::clr::Green);
  }
  void lister::kw_style() noexcept
  {
    reset_style();
    fmt::add_clr(out(), fmt::clr::Cyan);
  }
  void lister::lit_style() noexcept
  {
    reset_style();
    fmt::add_clr(out(), fmt::clr::Yellow);
  }
  void lister::default_style() noexcept
  {
    reset_style();
  }
  void lister::comment_style() noexcept
  {
    reset_style();
    fmt::add_clr(out(), fmt::clr::DarkGray);
  }
}