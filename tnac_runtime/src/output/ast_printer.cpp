#include "output/ast_printer.hpp"
#include "sema/sym/symbols.hpp"
#include "output/formatting.hpp"
#include "eval/value/value.hpp"

namespace tnac::rt::out
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

  void ast_printer::visit(const ast::root& root) noexcept
  {
    indent();
    node_designator("<root>"sv);
    additional_info(root);
    endl();
    push_parent(root.modules().size());
  }

  void ast_printer::visit(const ast::import_dir& id) noexcept
  {
    indent();
    node_designator("<import: "sv);
    auto&& name = id.name();
    if (!name.empty())
    {
      for (auto last = name.back(); auto part : name)
      {
        auto partName = part->name();
        if (part != last)
        {
          node_value(partName);
          out() << '.';
        }
        else
        {
          module_name(partName);
        }
      }
    }
    node_designator(">"sv);
    additional_info(id);
    endl();
    if (auto alias = id.alias_name())
    {
      push_parent(1u);
      indent();
      node_designator("Alias "sv);
      out() << '\'';
      module_name(alias->name());
      out() << "' ";
      additional_info(*alias);
      endl();
    }
  }

  void ast_printer::visit(const ast::module_def& moduleDef) noexcept
  {
    indent();
    if (moduleDef.is_fake())
    {
      node_designator("<Fake module>"sv);
    }
    else
    {
      node_designator("<module: "sv);
      print_module_name(moduleDef);
      node_designator(">"sv);
    }
    additional_info(moduleDef);
    endl();
    push_parent(moduleDef.children().size() + moduleDef.param_count() + moduleDef.import_count());
  }

  void ast_printer::visit(const ast::scope& scope) noexcept
  {
    indent();
    node_designator("<scope>"sv);
    additional_info(scope);
    endl();
    push_parent(scope.children().size());
  }

  void ast_printer::visit(const ast::assign_expr& expr) noexcept
  {
    indent();
    node_designator("Assign expression"sv);
    print_token(expr.op());
    additional_info(expr);
    endl();
    push_parent(2u);
  }

  void ast_printer::visit(const ast::decl_expr& expr) noexcept
  {
    indent();
    node_designator("Declaration "sv);
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
    out() << "<VarName: "; 
    node_value(decl.name()); 
    out() << '>';
    additional_info(decl);
    endl();
  }

  void ast_printer::visit(const ast::param_decl& decl) noexcept
  {
    indent();
    out() << "<Function parameter: "; 
    node_value(decl.name());
    out() << '>';
    additional_info(decl);
    endl();
  }

  void ast_printer::visit(const ast::func_decl& decl) noexcept
  {
    out() << "<FuncName: ";
    node_value(decl.name());
    out() << '>';
    additional_info(decl);
    endl();
  }

  void ast_printer::visit(const ast::binary_expr& expr) noexcept
  {
    indent();
    auto&& op = expr.op();
    node_designator("Binary expression"sv);
    print_token(op);
    additional_info(expr);
    endl();

    push_parent(2u);
  }

  void ast_printer::visit(const ast::unary_expr& expr) noexcept
  {
    indent();
    node_designator("Unary expression"sv);
    print_token(expr.op());
    additional_info(expr);
    endl();
    push_parent(1u);
  }

  void ast_printer::visit(const ast::type_check_expr& expr) noexcept
  {
    indent();
    node_designator("Is type"sv);
    print_token(expr.type());
    additional_info(expr);
    endl();
    push_parent(1u);
  }

  void ast_printer::visit(const ast::type_resolve_expr& expr) noexcept
  {
    indent();
    node_designator("On type"sv);
    additional_info(expr);
    endl();
    push_parent(2u);
  }

  void ast_printer::visit(const ast::tail_expr& expr) noexcept
  {
    indent();
    node_designator("Tail expression"sv);
    additional_info(expr);
    endl();
    push_parent(1u);
  }

  void ast_printer::visit(const ast::array_expr& arr) noexcept
  {
    const auto size = arr.elements().size();
    indent();
    node_designator("Array expression "sv);
    out() << '[';
    fmt::print(out(), fmt::clr::Cyan, size);
    out() << "] ";
    additional_info(arr);
    endl();
    push_parent(size);
  }

  void ast_printer::visit(const ast::paren_expr& expr) noexcept
  {
    indent();
    node_designator("Paren expression"sv);
    additional_info(expr);
    endl();
    push_parent(1u);
  }

  void ast_printer::visit(const ast::abs_expr& expr) noexcept
  {
    indent();
    node_designator("Abs expression"sv);
    additional_info(expr);
    endl();
    push_parent(1u);
  }

  void ast_printer::visit(const ast::typed_expr& expr) noexcept
  {
    indent();
    node_designator("Typed expression"sv);
    print_token(expr.type_name());
    additional_info(expr);
    endl();
    push_parent(expr.args().size());
  }

  void ast_printer::visit(const ast::call_expr& expr) noexcept
  {
    indent();
    node_designator("Call expression"sv);
    additional_info(expr);
    endl();
    push_parent(expr.args().size() + 1);
  }

  void ast_printer::visit(const ast::cond_short& expr) noexcept
  {
    indent();
    node_designator("Short conditional"sv);

    auto childCount = 1u;
    const auto hasTrue  = expr.has_true();
    const auto hasFalse = expr.has_false();

    if (hasTrue || hasFalse)
      out() << ": ";

    if (hasTrue)
    {
      node_value("has-true"sv);
      ++childCount;
    }
    if (hasFalse)
    {
      if (hasTrue) out() << ", ";
      node_value("has-false"sv);
      ++childCount;
    }

    additional_info(expr);
    endl();
    push_parent(childCount);
  }

  void ast_printer::visit(const ast::cond_expr& cond) noexcept
  {
    indent();
    node_designator("Conditional expression"sv);
    additional_info(cond);
    endl();
    push_parent(2u);
  }

  void ast_printer::visit(const ast::dot_expr& dot) noexcept
  {
    indent();
    node_designator("Dot expression"sv);
    additional_info(dot);
    endl();
    push_parent(2u);
  }

  void ast_printer::visit(const ast::pattern& pat) noexcept
  {
    indent();
    node_designator("Pattern"sv);
    additional_info(pat);
    endl();
    push_parent(2u);
  }

  void ast_printer::visit(const ast::matcher& matcher) noexcept
  {
    indent();
    if (matcher.is_default())
    {
      node_value("default"sv);
    }
    else if (matcher.is_unary())
    {
      out() << '{';
      node_value(matcher.pos().value());
      out() << '}';
    }
    else if (matcher.has_implicit_op())
    {
      push_parent(1);
      out() << '{'; 
      node_value("=="sv);
      out() << '}';
    }
    else
    {
      push_parent(1);
      out() << '{'; 
      node_value(matcher.pos().value());
      out() << '}';
    }

    additional_info(matcher);
    endl();
  }

  void ast_printer::visit(const ast::lit_expr& expr) noexcept
  {
    indent();
    node_designator("Literal expression"sv);
    print_token(expr.pos());
    additional_info(expr);
    endl();
  }

  void ast_printer::visit(const ast::id_expr& expr) noexcept
  {
    indent();
    node_designator("Id expression "sv);
    out() << '\'';
    node_value(expr.name());
    out() << "' ";
    additional_info(expr);
    endl();
  }

  void ast_printer::visit(const ast::result_expr& expr) noexcept
  {
    indent();
    node_designator("Last eval result "sv);
    additional_info(expr);
    endl();
  }

  void ast_printer::visit(const ast::ret_expr& expr) noexcept
  {
    indent();
    node_designator("Ret expression"sv);
    additional_info(expr);
    endl();
    push_parent(1u);
  }

  void ast_printer::visit(const ast::error_expr& expr) noexcept
  {
    indent();
    auto msg = expr.message();
    failure_condition("Error '"sv);
    failure_condition(msg); 
    failure_condition("' at"sv);
    location_info(expr.at().at());
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

  void ast_printer::node_designator(string_t str) noexcept
  {
    fmt::print(out(), fmt::clr::DarkYellow, str);
  }

  void ast_printer::failure_condition(string_t str) noexcept
  {
    fmt::print(out(), fmt::clr::Red, str);
  }

  void ast_printer::node_value(string_t str) noexcept
  {
    fmt::print(out(), fmt::clr::Cyan, str);
  }

  void ast_printer::module_name(string_t str) noexcept
  {
    fmt::print(out(), fmt::clr::DarkCyan, str);
  }

  void ast_printer::invalid_mark(const ast::node& n) noexcept
  {
    if (!n.is_valid())
      failure_condition(" has-errors");
  }

  void ast_printer::location_info(src::loc_wrapper loc) noexcept
  {
    fmt::add_clr(out(), fmt::clr::DarkGray);
    out() << " (" << loc << ')';
    fmt::clear_clr(out());
  }

  void ast_printer::additional_info(const ast::node& n) noexcept
  {
    invalid_mark(n);
  }

  void ast_printer::additional_info(const ast::expr& e) noexcept
  {
    location_info(e.pos().at());
    invalid_mark(e);
  }

  void ast_printer::additional_info(const ast::decl& d) noexcept
  {
    location_info(d.pos().at());
    invalid_mark(d);
  }

  void ast_printer::additional_info(const ast::import_dir& d) noexcept
  {
    location_info(d.pos().at());
    invalid_mark(d);
  }

  void ast_printer::endl() noexcept
  {
    out() << '\n';
  }

  out_stream& ast_printer::out() noexcept
  {
    return *m_out;
  }

  void ast_printer::print_token(const token& tok) noexcept
  {
    out() << " '"; 
    node_value(tok.value());
    out() << "' ";
  }

  void ast_printer::print_value(const eval::value& v) noexcept
  {
    out() << "<value: ";
    value_printer{}(v, 10, out());
    out() << '>';
  }

  void ast_printer::print_module_name(const ast::module_def& moduleDef) noexcept
  {
    auto&& sym = moduleDef.symbol();
    std::vector<string_t> nameParts;

    auto symScope = &sym.owner_scope();
    while (symScope)
    {
      auto scopeRef = symScope->to_scope_ref();
      if (scopeRef)
      {
        nameParts.emplace_back(scopeRef->name());
      }
      else if (auto mod = symScope->to_module())
      {
        nameParts.emplace_back(mod->name());
      }
      symScope = symScope->enclosing();
    }

    for (auto part : nameParts | views::reverse)
    {
      node_value(part);
      out() << '.';
    }

    module_name(moduleDef.name());
  }
}