#include "output/printer.hpp"
#include "evaluator/value.hpp"

namespace tnac_rt::out
{
  // AST printer

  // Special members

  ast_printer::~ast_printer() noexcept = default;

  ast_printer::ast_printer() noexcept = default;

  // Public members

  void ast_printer::operator()(const ast::node* node, out_stream& os) noexcept
  {
    m_out = &os;
    base::operator()(node);
  }

  // Private members

  void ast_printer::push_parent(child_count childCount) noexcept
  {
    m_indetations.push({ .m_childIdx{ childCount }, .m_depth{ m_curDepth + indentStep } });
  }

  void ast_printer::indent() noexcept
  {
    if (m_indetations.empty())
      return;

    auto&& curIndent = m_indetations.top();
    m_curDepth = curIndent.m_depth;

    for (auto depth = m_curDepth; depth > indentStep; depth -= indentStep)
      out() << "| ";

    if (!--curIndent.m_childIdx)
    {
      out() << '`';
      m_indetations.pop();
    }
    else
    {
      out() << '|';
    }

    out() << '-';
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

  void ast_printer::visit(const ast::scope* scope) noexcept
  {
    indent();
    out() << "<scope>";
    endl();
    push_parent(scope->children().size());
  }

  void ast_printer::visit(const ast::assign_expr* expr) noexcept
  {
    indent();
    out() << "Assign expression";
    print_token(expr->op());
    print_value(expr->value());
    endl();
    push_parent(2u);
  }

  void ast_printer::visit(const ast::decl_expr* expr) noexcept
  {
    indent();
    out() << "Declaration ";
    print_value(expr->value());
    push_parent(1u);
  }

  void ast_printer::visit(const ast::var_decl* decl) noexcept
  {
    out() << " <VarName: " << decl->name() << ">";
    endl();
  }

  void ast_printer::visit(const ast::binary_expr* expr) noexcept
  {
    indent();
    auto&& op = expr->op();
    out() << "Binary expression";

    print_token(op);
    print_value(expr->value());
    endl();

    push_parent(2u);
  }

  void ast_printer::visit(const ast::unary_expr* expr) noexcept
  {
    indent();
    out() << "Unary expression";
    print_token(expr->op());
    print_value(expr->value());
    endl();
    push_parent(1u);
  }

  void ast_printer::visit(const ast::paren_expr* expr) noexcept
  {
    indent();
    out() << "Paren expression";
    print_value(expr->value());
    endl();
    push_parent(1u);
  }

  void ast_printer::visit(const ast::lit_expr* expr) noexcept
  {
    indent();
    out() << "Literal expression";
    print_token(expr->pos());
    print_value(expr->value());
    endl();
  }

  void ast_printer::visit(const ast::id_expr* expr) noexcept
  {
    indent();
    out() << "Id expression '";
    out() << expr->name() << "' ";
    print_value(expr->value());
    endl();
  }

  void ast_printer::visit(const ast::error_expr* expr) noexcept
  {
    indent();
    auto&& at = expr->at();
    auto msg = expr->message();
    out() << "Error '" << msg << "' at";
    print_token(at);
    endl();
  }
}