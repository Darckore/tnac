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
    kw_string(bb.name());
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
    using enum ir::op_code;
    switch (instr.opcode())
    {
    case Add:
    case Sub:
    case Mul:
    case Div:
    case Mod:
    case Pow:
    case Root:
    case And:
    case Or:
    case Xor:
    case CmpE:
    case CmpL:
    case CmpLE:
    case CmpNE:
    case CmpG:
    case CmpGE:
      print_binary(instr);
      break;

    case Abs:
    case CmpNot:
    case CmpIs:
    case Plus:
    case Neg:
    case BNeg:
      print_unary(instr);
      break;

    case Alloc: print_alloc(instr); break;
    case Store: print_store(instr); break;
    case Load:  print_load(instr);  break;
    case Call:  break;
    case Jump:  print_jump(instr);  break;
    case Ret:   print_ret(instr);   break;

    case Phi: break;
    }
    endl();
  }


  // Private members

  void ir_printer::print_operand(const ir::operand& op) noexcept
  {
    if (op.is_value())
      value(op.get_value());
    else if (op.is_register())
      vreg(op.get_reg());
    else if (op.is_param())
      param(op.get_param());
    else if (op.is_block())
      block(op.get_block());
  }

  void ir_printer::print_assign(const ir::operand& op) noexcept
  {
    print_operand(op);
    out() << " = ";
  }

  void ir_printer::print_alloc(const ir::instruction& alloc) noexcept
  {
    print_assign(alloc[0]);
    keyword(alloc.opcode_str());
  }

  void ir_printer::print_store(const ir::instruction& store) noexcept
  {
    keyword(store.opcode_str());
    print_operand(store[0]);
    out() << ", ";
    print_operand(store[1]);
  }

  void ir_printer::print_load(const ir::instruction& load) noexcept
  {
    print_assign(load[0]);
    keyword(load.opcode_str());
    print_operand(load[1]);
  }

  void ir_printer::print_binary(const ir::instruction& bin) noexcept
  {
    print_assign(bin[0]);
    keyword(bin.opcode_str());
    print_operand(bin[1]);
    out() << ", ";
    print_operand(bin[2]);
  }

  void ir_printer::print_unary(const ir::instruction& un) noexcept
  {
    print_assign(un[0]);
    keyword(un.opcode_str());
    print_operand(un[1]);
  }

  void ir_printer::print_ret(const ir::instruction& ret) noexcept
  {
    keyword(ret.opcode_str());
    print_operand(ret[0]);
  }

  void ir_printer::print_jump(const ir::instruction& jmp) noexcept
  {
    keyword(jmp.opcode_str());
    print_operand(jmp[0]);

    if (jmp.operand_count() < 3)
      return;

    out() << ", ";
    print_operand(jmp[1]);
    out() << ", ";
    print_operand(jmp[2]);
  }


  out_stream& ir_printer::out() noexcept
  {
    return *m_out;
  }

  void ir_printer::endl() noexcept
  {
    out() << '\n';
  }

  void ir_printer::kw_string(string_t kw) noexcept
  {
    fmt::print(out(), fmt::clr::BoldBlue, kw);
  }

  void ir_printer::keyword(string_t kw) noexcept
  {
    kw_string(kw);
    out() << ' ';
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
    keyword(val.id_str());
    auto visitor = utils::visitor
    {
      [&](eval::invalid_val_t) noexcept
      {},
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

  void ir_printer::vreg(const ir::vreg& reg) noexcept
  {
    out() << '%';
    if (reg.is_named())
      out() << reg.name();
    else
      out() << reg.index();
  }

  void ir_printer::block(const ir::basic_block& block) noexcept
  {
    keyword("label"sv);
    name("%"sv);
    name(block.name());
  }

  void ir_printer::param(ir::func_param par) noexcept
  {
    keyword("param"sv);
    fmt::print(out(), fmt::clr::BoldYellow, *par);
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
    keyword("declare"sv);
    func_intro(fn);
    endl();
  }

  void ir_printer::func_intro(const ir::function& fn) noexcept
  {
    if (fn.owner_func())
      keyword("function"sv);
    else
      keyword("module"sv);

    id(fn.id());
    name(" @"sv);
    name(fn.name());
  }

}