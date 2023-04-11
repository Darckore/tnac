#include "output/printer.hpp"
#include "evaluator/value.hpp"

namespace tnac_rt::out
{
  // AST printer

  // Special members

  ast_printer::~ast_printer() noexcept
  {
    using sz_t = child_tracker::size_type;
    static constexpr auto initialSize = sz_t{ 16 };
    m_indetations.reserve(initialSize);
  }

  ast_printer::ast_printer() noexcept = default;

  // Public members

  void ast_printer::operator()(const ast::node* node, out_stream& os) noexcept
  {
    m_indetations.clear();
    m_out = &os;
    base::operator()(node);
  }

  void ast_printer::operator()(const ast::node* node) noexcept
  {
    this->operator()(node, *m_out);
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

    for (auto&& last = last_indent(); auto&& cur : m_indetations)
    {
      if (&cur != &last)
      {
        print_parent(cur);
        continue;
      }

      print_child(last);
    }
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
    out() << " '" << tok.m_value << "' ";
  }

  namespace
  {
    inline std::ostream& operator<<(std::ostream& out, tnac::eval::invalid_val_t) noexcept
    {
      out << "<undef>";
      return out;
    }
  }

  void ast_printer::print_value(tnac::eval::value v) noexcept
  {
    tnac::eval::on_value(v, [this, addr = v.raw_value()](auto val)
      {
        out() << "(0x" << std::hex << addr << std::dec << ": " << val << ")";
      });
  }


  // Public members

  void ast_printer::visit(const ast::scope& scope) noexcept
  {
    indent();
    out() << "<scope>";
    endl();
    push_parent(scope.children().size());
  }

  void ast_printer::visit(const ast::assign_expr& expr) noexcept
  {
    indent();
    out() << "Assign expression";
    print_token(expr.op());
    print_value(expr.value());
    endl();
    push_parent(2u);
  }

  void ast_printer::visit(const ast::decl_expr& expr) noexcept
  {
    indent();
    out() << "Declaration ";
    print_value(expr.value());
    push_parent(1u);
  }

  void ast_printer::visit(const ast::var_decl& decl) noexcept
  {
    out() << " <VarName: " << decl.name() << ">";
    endl();
  }

  void ast_printer::visit(const ast::binary_expr& expr) noexcept
  {
    indent();
    auto&& op = expr.op();
    out() << "Binary expression";

    print_token(op);
    print_value(expr.value());
    endl();

    push_parent(2u);
  }

  void ast_printer::visit(const ast::unary_expr& expr) noexcept
  {
    indent();
    out() << "Unary expression";
    print_token(expr.op());
    print_value(expr.value());
    endl();
    push_parent(1u);
  }

  void ast_printer::visit(const ast::paren_expr& expr) noexcept
  {
    indent();
    out() << "Paren expression";
    print_value(expr.value());
    endl();
    push_parent(1u);
  }

  void ast_printer::visit(const ast::lit_expr& expr) noexcept
  {
    indent();
    out() << "Literal expression";
    print_token(expr.pos());
    print_value(expr.value());
    endl();
  }

  void ast_printer::visit(const ast::id_expr& expr) noexcept
  {
    indent();
    out() << "Id expression '";
    out() << expr.name() << "' ";
    print_value(expr.value());
    endl();
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
}