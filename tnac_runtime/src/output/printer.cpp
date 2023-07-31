#include "output/printer.hpp"

namespace tnac_rt::out
{
  // Special members

  ast_printer::~ast_printer() noexcept = default;

  ast_printer::ast_printer() noexcept
  {
    using sz_t = child_tracker::size_type;
    static constexpr auto initialSize = sz_t{ 16 };
    m_indetations.reserve(initialSize);
  }

  // Public members

  void ast_printer::operator()(const ast::node* node, out_stream& os) noexcept
  {
    m_indetations.clear();
    m_out = &os;
    base::operator()(node);
  }

  void ast_printer::operator()(const ast::node* node) noexcept
  {
    operator()(node, out());
  }

  void ast_printer::visit(const ast::scope& scope) noexcept
  {
    indent();
    out() << "<scope>";
    invalid_mark(scope);
    endl();
    push_parent(scope.children().size());
  }

  void ast_printer::visit(const ast::assign_expr& expr) noexcept
  {
    indent();
    out() << "Assign expression";
    print_token(expr.op());
    invalid_mark(expr);
    endl();
    push_parent(2u);
  }

  void ast_printer::visit(const ast::decl_expr& expr) noexcept
  {
    indent();
    out() << "Declaration ";
    invalid_mark(expr);
    auto&& decl = expr.declarator();

    if (decl.is(ast::node::VarDecl))
    {
      push_parent(1u);
    }
    else if (decl.is(ast::node::FuncDecl))
    {
      auto&& funcDecl = utils::cast<ast::func_decl>(decl);
      push_parent(funcDecl.param_count() + 1u);
    }
  }

  void ast_printer::visit(const ast::var_decl& decl) noexcept
  {
    out() << " <VarName: " << decl.name() << '>';
    invalid_mark(decl);
    endl();
  }

  void ast_printer::visit(const ast::param_decl& decl) noexcept
  {
    indent();
    out() << " <Function parameter: " << decl.name() << '>';
    invalid_mark(decl);
    endl();
  }

  void ast_printer::visit(const ast::func_decl& decl) noexcept
  {
    out() << "<FuncName: " << decl.name() << '>';
    invalid_mark(decl);
    endl();
  }

  void ast_printer::visit(const ast::binary_expr& expr) noexcept
  {
    indent();
    auto&& op = expr.op();
    out() << "Binary expression";

    print_token(op);
    invalid_mark(expr);
    endl();

    push_parent(2u);
  }

  void ast_printer::visit(const ast::unary_expr& expr) noexcept
  {
    indent();
    out() << "Unary expression";
    print_token(expr.op());
    invalid_mark(expr);
    endl();
    push_parent(1u);
  }

  void ast_printer::visit(const ast::array_expr& arr) noexcept
  {
    const auto size = arr.elements().size();
    indent();
    out() << "Array expression [" << size << "]";
    invalid_mark(arr);
    endl();
    push_parent(size);
  }

  void ast_printer::visit(const ast::paren_expr& expr) noexcept
  {
    indent();
    out() << "Paren expression";
    invalid_mark(expr);
    endl();
    push_parent(1u);
  }

  void ast_printer::visit(const ast::abs_expr& expr) noexcept
  {
    indent();
    out() << "Abs expression";
    invalid_mark(expr);
    endl();
    push_parent(1u);
  }

  void ast_printer::visit(const ast::typed_expr& expr) noexcept
  {
    indent();
    out() << "Typed expression";
    print_token(expr.type_name());
    invalid_mark(expr);
    endl();
    push_parent(expr.args().size());
  }

  void ast_printer::visit(const ast::call_expr& expr) noexcept
  {
    indent();
    out() << "Call expression";
    invalid_mark(expr);
    endl();
    push_parent(expr.args().size() + 1);
  }

  void ast_printer::visit(const ast::cond_short& expr) noexcept
  {
    indent();
    out() << "Short conditional";

    auto childCount = 1u;
    const auto hasTrue  = expr.has_true();
    const auto hasFalse = expr.has_false();

    if (hasTrue || hasFalse)
      out() << ": ";

    if (hasTrue)
    {
      out() << "has true";
      ++childCount;
    }
    if (hasFalse)
    {
      if (hasTrue) out() << ", ";
      out() << "has false";
      ++childCount;
    }

    invalid_mark(expr);
    endl();
    push_parent(childCount);
  }

  void ast_printer::visit(const ast::cond_expr& cond) noexcept
  {
    indent();
    out() << "Conditional expression";
    invalid_mark(cond);
    endl();
    push_parent(2u);
  }

  void ast_printer::visit(const ast::pattern& pat) noexcept
  {
    indent();
    out() << "Pattern";
    invalid_mark(pat);
    endl();
    push_parent(2u);
  }

  void ast_printer::visit(const ast::matcher& matcher) noexcept
  {
    indent();
    if (matcher.is_default())
    {
      out() << "default";
    }
    else if (matcher.is_unary())
    {
      out() << '{' << matcher.pos().value() << '}';
    }
    else if (matcher.has_implicit_op())
    {
      push_parent(1);
      out() << "{==}";
    }
    else
    {
      push_parent(1);
      out() << '{' << matcher.pos().value() << '}';
    }

    invalid_mark(matcher);
    endl();
  }

  void ast_printer::visit(const ast::lit_expr& expr) noexcept
  {
    indent();
    out() << "Literal expression";
    print_token(expr.pos());
    endl();
  }

  void ast_printer::visit(const ast::id_expr& expr) noexcept
  {
    indent();
    out() << "Id expression '";
    out() << expr.name() << "' ";
    endl();
  }

  void ast_printer::visit(const ast::result_expr& ) noexcept
  {
    indent();
    out() << "Last eval result ";
    endl();
  }

  void ast_printer::visit(const ast::ret_expr& expr) noexcept
  {
    indent();
    out() << "Ret expression";
    invalid_mark(expr);
    endl();
    push_parent(1u);
  }

  void ast_printer::visit(const ast::error_expr& expr) noexcept
  {
    indent();
    auto&& at = expr.at();
    auto msg = expr.message();
    out() << "Error '" << msg << "' at";
    print_token(at);
    endl();
  }

  // Private members

  void ast_printer::push_parent(child_count childCount) noexcept
  {
    m_indetations.push_back(childCount);
  }

  void ast_printer::pop_empty() noexcept
  {
    while (!m_indetations.empty())
    {
      if (m_indetations.back())
        break;

      m_indetations.pop_back();
    }
  }

  ast_printer::child_count& ast_printer::last_indent() noexcept
  {
    auto&& last = m_indetations.back();
    --last;
    return last;
  }

  void ast_printer::print_parent(child_count cur) noexcept
  {
    static constexpr auto childIndent = "| "sv;
    static constexpr auto blankIndent = "  "sv;

    if (!cur)
      out() << blankIndent;
    else
      out() << childIndent;
  }

  void ast_printer::print_child(child_count cur) noexcept
  {
    static constexpr auto prefix = "|-"sv;
    static constexpr auto lastPrefix = "`-"sv;

    if (!cur)
      out() << lastPrefix;
    else
      out() << prefix;
  }

  void ast_printer::indent() noexcept
  {
    pop_empty();

    if (m_indetations.empty())
      return;

    for (auto&& last = last_indent(); auto && cur : m_indetations)
    {
      if (&cur != &last)
      {
        print_parent(cur);
        continue;
      }

      print_child(last);
    }
  }

  void ast_printer::invalid_mark(const ast::node& n) noexcept
  {
    if (n.is_valid())
      return;
    out() << " !! invalid !! ";
  }

  void ast_printer::endl() noexcept
  {
    out() << '\n';
  }

  out_stream& ast_printer::out() noexcept
  {
    return *m_out;
  }

  void ast_printer::print_token(const tnac::token& tok) noexcept
  {
    out() << " '" << tok << "' ";
  }

  void ast_printer::print_value(eval::value v) noexcept
  {
    out() << "<value: ";
    value_printer{}(v, 10, out());
    out() << '>';
  }
}