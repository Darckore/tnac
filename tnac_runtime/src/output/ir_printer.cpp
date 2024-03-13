#include "output/ir_printer.hpp"
#include "output/formatting.hpp"

namespace tnac::rt::out
{
  // Special members

  ir_printer::~ir_printer() noexcept = default;

  ir_printer::ir_printer() noexcept = default;


  // Public members

  void ir_printer::operator()(const ir::cfg& gr, out_stream& os) noexcept
  {
    m_out = &os;
    declare_funcs(gr);
    base::operator()(gr);
  }

  void ir_printer::operator()(const ir::cfg& gr) noexcept
  {
    operator()(gr, out());
  }

  bool ir_printer::preview(const ir::function& fn) noexcept
  {
    func_intro(fn);
    endl();
    return true;
  }

  bool ir_printer::preview(const ir::basic_block& bb) noexcept
  {
    keyword(bb.name());
    plain(":"sv);
    endl();
    return true;
  }

  void ir_printer::visit(const ir::function&) noexcept
  {
    keyword("end"sv);
    endl();
    endl();
  }

  void ir_printer::visit(const ir::basic_block&) noexcept
  {
    endl();
  }

  void ir_printer::visit(const ir::instruction& instr) noexcept
  {
    out() << "  ";
    keyword(instr.opcode_str());
    out() << ' ';
    using enum ir::op_code;
    switch (instr.opcode())
    {
    case Add:   break;
    case Sub:   break;
    case Mul:   break;
    case Div:   break;
    case Mod:   break;
    case Pow:   break;
    case Root:  break;
    case And:   break;
    case Or:    break;
    case Xor:   break;
    case CmpE:  break;
    case CmpL:  break;
    case CmpLE: break;

    case Abs:   break;

    case Store: break;
    case Load:  break;
    case Call:  break;
    case Jump:  break;
    case Ret:   print_ret(instr); break;
    }
    endl();
  }


  // Private members

  void ir_printer::print_operand(const ir::operand& op) noexcept
  {
    if (op.is_value())
      value(op.get_value());
  }

  void ir_printer::print_ret(const ir::instruction& ret) noexcept
  {
    auto&& op = ret[0];
    print_operand(op);
  }


  out_stream& ir_printer::out() noexcept
  {
    return *m_out;
  }

  void ir_printer::endl() noexcept
  {
    out() << '\n';
  }

  void ir_printer::keyword(string_t kw) noexcept
  {
    fmt::print(out(), fmt::clr::BoldBlue, kw);
  }

  void ir_printer::name(string_t n) noexcept
  {
    fmt::print(out(), fmt::clr::BoldCyan, n);
  }

  void ir_printer::id(entity_id i) noexcept
  {
    fmt::print(out(), fmt::clr::Yellow, i);
  }

  void ir_printer::value(eval::value val) noexcept
  {
    auto visitor = utils::visitor
    {
      [&](eval::invalid_val_t) noexcept
      {
        keyword("undef"sv);
      },
      [&](eval::bool_type v) noexcept
      {
        if (v) keyword("true"sv);
        else keyword("false"sv);
      },
      [&](eval::array_type) noexcept
      {
        // todo: arrays
      },
      [&](auto v) noexcept
      {
        fmt::print(out(), fmt::clr::BoldYellow, v);
      }
    };

    eval::on_value(val, visitor);
  }

  void ir_printer::plain(string_t str) noexcept
  {
    out() << str;
  }

  void ir_printer::declare_funcs(const ir::cfg& gr) noexcept
  {
    std::queue<ir::function*> fnq;
    for (auto mod : gr)
    {
      fnq.push(mod);
    }

    while (!fnq.empty())
    {
      auto fn = fnq.front();
      fnq.pop();
      declare(*fn);

      for (auto child : fn->children())
      {
        fnq.push(child);
      }
    }

    endl();
  }

  void ir_printer::declare(const ir::function& fn) noexcept
  {
    keyword("declare "sv);
    func_intro(fn);
    endl();
  }

  void ir_printer::func_intro(const ir::function& fn) noexcept
  {
    if (fn.owner_func())
      keyword("function "sv);
    else
      keyword("module "sv);

    id(fn.id());
    name(" @"sv);
    name(fn.name());
  }

}