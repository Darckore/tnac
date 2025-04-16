#include "input/repl.hpp"
#include "output/ast_printer.hpp"
#include "output/sym_printer.hpp"
#include "output/lister.hpp"
#include "output/ir_printer.hpp"
#include "common/feedback.hpp"
#include "common/diag.hpp"
#include "sema/sym/symbols.hpp"
#include "src_mgr/source_manager.hpp"
#include "parser/ast/ast.hpp"

namespace tnac::rt::detail
{
  namespace
  {
    string_t get_module_name(const ir::function& mod) noexcept
    {
      auto name = utils::split(mod.name(), ":"sv);
      return *name.begin();
    }
    bool is_top_level(string_t name) noexcept
    {
      const auto dot = name.find_first_of('.');
      return dot == string_t::npos;
    }
  }
}

namespace tnac::rt
{
  // Special members

  repl::~repl() noexcept = default;

  repl::repl(state& st, feedback& fb) noexcept :
    m_loc{ m_fake, m_srcMgr },
    m_state{ &st },
    m_feedback{ &fb }
  {}


  // Public members

  void repl::run() noexcept
  {
    m_state->start();
    init_modules();

    auto&& core = m_state->tnac_core();
    auto&& replMod = *m_replMod;
    auto&& ev = core.ir_evaluator();

    using enum ir::op_code;
    using block_set = std::unordered_set<const ir::basic_block*>;
    block_set blocks;

    auto&& entry = replMod.entry();
    auto curBlock = &entry;
    auto lastInstr = entry.begin();
    auto lastAlloc = lastInstr;

    auto runFunc = [&](auto term) noexcept
      {
        for (;;)
        {
          ev.step();
          if (ev.instr_ptr() == term)
            break;
        }
      };

    auto evalAllocs = [&]() noexcept
      {
        if (!lastAlloc)
        {
          lastAlloc = entry.begin();
          if (!lastAlloc)
            return;
          if (utils::eq_none(lastAlloc->opcode(), Alloc, Arr))
          {
            lastAlloc = {};
            return;
          }
          ev.init_instr_ptr(*lastAlloc);
          ev.step();
        }

        for (;;)
        {
          auto next = lastAlloc->next();
          if (!next)
            return;
          if (utils::eq_any(next->opcode(), Alloc, Arr))
          {
            ev.init_instr_ptr(*next);
            ev.step();
            lastAlloc = next->to_iterator();
            continue;
          }
          return;
        }
      };

    bool nullRetAddr = false;
    while (m_state->is_running())
    {
      auto input = consume_input();
      if (input.empty())
        continue;

      auto parseRes = core.parse(input, m_loc);
      if (parseRes == core.get_ast())
        continue;

      m_last = parseRes;
      core.compile(*m_last);

      evalAllocs();
      bool hasNew = false;
      if (!lastInstr)
      {
        lastInstr = entry.begin();
        hasNew = static_cast<bool>(lastInstr);
      }
      else if (nullRetAddr)
      {
        auto next = lastInstr->next();
        hasNew = static_cast<bool>(next);
        if (hasNew)
          lastInstr = next->to_iterator();
      }
      else
      {
        hasNew = lastInstr != curBlock->last();
      }

      if (!hasNew)
      {
        print_result();
        continue;
      }

      ev.init_instr_ptr(*lastInstr);
      for (;;)
      {
        const auto oc = lastInstr->opcode();
        if (oc == Call)
        {
          auto retAddr = lastInstr->next();
          nullRetAddr = !static_cast<bool>(retAddr);
          runFunc(retAddr);
        }
        else
        {
          nullRetAddr = false;
          ev.step();
        }

        auto iptr = ev.instr_ptr();
        if (!iptr)
          break;

        if (auto block = &iptr->owner_block(); block != &lastInstr->owner_block())
        {
          if (oc != Jump || blocks.find(block) != blocks.end())
            break;

          blocks.emplace(block);
          curBlock = block;
        }
        lastInstr = iptr->to_iterator();
      }

      print_value(ev.result());
    }
  }

  void repl::declare_commands() noexcept
  {
    if (m_commandsReady)
      return;

    using enum tok_kind;
    using params = commands::descr::param_list;
    using size_type = params::size_type;
    auto&& core = m_state->tnac_core();

    core.declare_cmd("exit"sv, [this](auto) noexcept { on_exit(); });
    core.declare_cmd("result"sv, params{ Identifier }, size_type{},
         [this](auto c) noexcept { print_result(std::move(c)); });

    core.declare_cmd("list"sv, params{ String }, size_type{},
         [this](auto c) noexcept { list_code(std::move(c)); });

    core.declare_cmd("ast"sv, params{ String, Identifier }, size_type{},
         [this](auto c) noexcept { print_ast(std::move(c)); });

    core.declare_cmd("ir"sv, params{ String }, size_type{},
         [this](auto c) noexcept { print_ir(std::move(c)); });

    core.declare_cmd("vars"sv, params{ String }, size_type{},
         [this](auto c) noexcept { print_vars(std::move(c)); });

    core.declare_cmd("funcs"sv, params{ String }, size_type{},
         [this](auto c) noexcept { print_funcs(std::move(c)); });

    core.declare_cmd("modules"sv, params{ String }, size_type{},
         [this](auto c) noexcept { print_modules(std::move(c)); });

    core.declare_cmd("env"sv, params{ String }, size_type{},
         [this](auto c) noexcept { print_all(std::move(c)); });

    core.declare_cmd("bin"sv, [this](auto) noexcept { m_state->set_base(2); });
    core.declare_cmd("oct"sv, [this](auto) noexcept { m_state->set_base(8); });
    core.declare_cmd("dec"sv, [this](auto) noexcept { m_state->set_base(10); });
    core.declare_cmd("hex"sv, [this](auto) noexcept { m_state->set_base(16); });

    m_commandsReady = true;
  }

  void repl::on_command(ast::command cmd) noexcept
  {
    m_state->tnac_core().process_cmd(std::move(cmd));
  }

  repl::line_opt repl::fetch_line(src::loc_wrapper loc) noexcept
  {
    if (!loc || &loc->file() != &m_fake)
      return {};

    auto inputIt = m_inputs.find(loc->line());
    if (inputIt == m_inputs.end())
      return {};

    return inputIt->second;
  }


  // Private members

  void repl::init_modules() noexcept
  {
    auto&& core = m_state->tnac_core();
    auto astRoot = utils::try_cast<ast::root>(core.parse({}, m_loc));
    UTILS_ASSERT(astRoot);

    auto&& modules = astRoot->modules();
    UTILS_ASSERT(!modules.empty());

    auto replMod = astRoot->modules().back();
    UTILS_ASSERT(replMod->name() == "REPL"sv);

    auto&& sema = core.get_sema();
    auto scg = sema.assume_scope(replMod->symbol().own_scope());

    auto&& cmp = core.get_compiler();
    auto&& cfg = cmp.cfg();
    auto&& replIr = cfg.declare_module(&replMod->symbol(), replMod->name(), {});
    for (auto mod : modules)
    {
      auto curMod = cfg.find_entity(&mod->symbol());
      if (!curMod || curMod == &replIr)
        break;

      auto modName = detail::get_module_name(*curMod);
      if (!detail::is_top_level(modName))
        continue;

      sema.visit_import_alias(*mod);
      replIr.add_child_name(*curMod);
    }

    cmp.attach_module(replIr, *replMod);
    auto&& ev = core.ir_evaluator();
    ev.enter(replIr);
    m_replMod = &replIr;
  }

  string_t repl::consume_input() noexcept
  {
    buf_t input;

    fmt::print(m_state->out(), fmt::clr::Yellow, ">> "sv);
    std::getline(m_state->in(), input);
    if (utils::ltrim(input).empty())
    {
      fmt::println(m_state->out(), fmt::clr::Red, "\nEnter an expression\n"sv);
      return {};
    }

    auto&& newItem = m_inputs.try_emplace(m_loc.line(), std::move(input));
    return newItem.first->second;
  }

  bool repl::try_redirect_output(const token& path) noexcept
  {
    std::error_code errc;
    auto fileName = fsys::absolute(fsys::path{ path.value() }, errc);
    if (fileName.empty())
      return false;

    if (!m_state->redirect_to_file(fileName))
    {
      m_feedback->compile_error(path.at(), diag::file_write_failure(fileName, "not accessible"sv));

      return false;
    }

    return true;
  }

  void repl::end_redirect() noexcept
  {
    m_state->reset_output();
  }


  // Private members (Command handlers)

  void repl::on_exit() noexcept
  {
    fmt::println(m_state->out(), fmt::clr::Yellow, "\nGoody-bye"sv);
    m_state->stop();
  }

  void repl::print_value(const eval::value& val) noexcept
  {
    auto&& os = m_state->out();
    fmt::println(os, fmt::clr::Yellow, "Result: "sv);
    fmt::add_clr(os, fmt::clr::White);
    out::value_printer{}(val, m_state->num_base(), os);
    fmt::clear_clr(os);
    os << '\n';
  }

  void repl::print_result() noexcept
  {
    auto&& core = m_state->tnac_core();
    auto&& compiler = core.get_compiler();
    auto&& ev = core.ir_evaluator();
    if (auto reg = compiler.peek_reg())
      ev.try_load(*reg);

    if (auto val = compiler.peek_value())
      print_value(*val);
    else
      print_value(ev.result());
  }

  template <std::invocable<> F>
  void repl::print_cmd(const ast::command& cmd, F&& printFunc) noexcept
  {
    using size_type = ast::command::size_type;
    auto wrapInLines = true;
    if (cmd.arg_count())
    {
      wrapInLines = !try_redirect_output(cmd[size_type{}]);
    }

    if (wrapInLines) m_state->out() << '\n';
    printFunc();
    if (wrapInLines) m_state->out() << '\n';
    end_redirect();
  }

  void repl::print_result(ast::command cmd) noexcept
  {
    using size_type = ast::command::size_type;
    static constexpr auto bin = "bin"sv;
    static constexpr auto oct = "oct"sv;
    static constexpr auto dec = "dec"sv;
    static constexpr auto hex = "hex"sv;

    auto base = m_state->num_base();
    const auto prevBase = base;
    if (cmd.arg_count())
    {
      auto&& arg = cmd[size_type{}];
      auto argName = arg.value();
      if (argName == bin)
        base = 2;
      else if (argName == oct)
        base = 8;
      else if (argName == dec)
        base = 10;
      else if (argName == hex)
        base = 16;
      else
        m_feedback->compile_error(arg.at(), diag::wrong_cmd_arg(size_type{}, arg.value()));
    }

    m_state->set_base(base);
    SCOPE_GUARD(m_state->set_base(prevBase));
    print_result();
  }

  void repl::list_code(ast::command cmd) noexcept
  {
    print_cmd(cmd, [this]
      {
        out::lister ls;
        ls(m_state->tnac_core().get_ast(), m_state->out());
      });
  }

  void repl::print_ast(ast::command cmd) noexcept
  {
    auto toPrint = [this](const ast::command& c) noexcept -> const tnac::ast::node*
      {
        using size_type = ast::command::size_type;
        constexpr auto maxArgs = size_type{ 2 };
        const auto argCount = c.arg_count();
        auto&& core = m_state->tnac_core();

        if (argCount < maxArgs)
          return core.get_ast();

        const auto curIdx = size_type{ 1 };
        auto&& second = c[curIdx];
        if (second.value() == "current"sv)
          return m_last;

        m_feedback->compile_error(second.at(), diag::wrong_cmd_arg(curIdx, second.value()));
        return core.get_ast();
      };

    auto ast = toPrint(cmd);
    print_cmd(cmd, [this, ast]
      {
        out::ast_printer pr;
        pr(ast, m_state->out());
      });
  }

  void repl::print_ir(ast::command cmd) noexcept
  {
    print_cmd(cmd, [this]
      {
        out::ir_printer ip;
        ip(m_state->tnac_core().get_cfg(), m_state->out());
      });
  }

  template <semantics::sem_symbol S>
  void repl::print_symbols(semantics::sym_container<S> collection) noexcept
  {
    out::sym_printer sp;
    sp(collection, m_state->out());
  }

  void repl::print_vars(ast::command cmd) noexcept
  {
    print_cmd(cmd, [this]
      {
        print_symbols(m_state->tnac_core().variables());
      });
  }

  void repl::print_funcs(ast::command cmd) noexcept
  {
    print_cmd(cmd, [this]
      {
        print_symbols(m_state->tnac_core().functions());
      });
  }

  void repl::print_modules(ast::command cmd) noexcept
  {
    print_cmd(cmd, [this]
      {
        print_symbols(m_state->tnac_core().modules());
      });
  }

  void repl::print_all(ast::command cmd) noexcept
  {
    using ast::command;
    using cmd_args = command::arg_list;
    auto loc = cmd.pos().at();
    auto cTok = [loc](string_t val) noexcept
      {
        return token{ val, token::Command, loc };
      };
    auto sTok = [loc](string_t val) noexcept
      {
        return token{ val, token::String, loc };
      };

    buf_t fAst;
    buf_t fList;
    buf_t fVars;
    buf_t fFuncs;
    buf_t fMods;
    buf_t fIr;

    cmd_args aAst;
    cmd_args aList;
    cmd_args aVars;
    cmd_args aFuncs;
    cmd_args aMods;
    cmd_args aIr;

    using size_type = ast::command::size_type;
    bool hasOut{};
    if (cmd.arg_count())
    {
      auto dir = cmd[size_type{}].value();
      if (std::error_code err; !fsys::create_directory(dir, err))
      {
        if (err)
        {
          m_feedback->error(diag::format("#env command failed with error '{}'"sv, err.message()));
          return;
        }
      }
      hasOut = true;
      fAst.append(dir).append("/out.ast");
      aAst.emplace_back(sTok(fAst));
      fList.append(dir).append("/out.tnac");
      aList.emplace_back(sTok(fList));
      fVars.append(dir).append("/tnvars");
      aVars.emplace_back(sTok(fVars));
      fFuncs.append(dir).append("/tnfuncs");
      aFuncs.emplace_back(sTok(fFuncs));
      fMods.append(dir).append("/tnmodules");
      aMods.emplace_back(sTok(fMods));
      fIr.append(dir).append("/out.tni");
      aIr.emplace_back(sTok(fIr));
    }

    if (!hasOut) m_state->out() << "==========  CODE   ==========\n";
    on_command({ cTok("list"sv), std::move(aList) });
    if (!hasOut) m_state->out() << "========== MODULES ==========\n";
    on_command({ cTok("modules"sv), std::move(aMods) });
    if (!hasOut) m_state->out() << "==========  FUNCS  ==========\n";
    on_command({ cTok("funcs"sv), std::move(aFuncs) });
    if (!hasOut) m_state->out() << "==========  VARS   ==========\n";
    on_command({ cTok("vars"sv), std::move(aVars) });
    if (!hasOut) m_state->out() << "==========  AST    ==========\n";
    on_command({ cTok("ast"sv), std::move(aAst) });
    if (!hasOut) m_state->out() << "==========  IR   ==========\n";
    on_command({ cTok("ir"sv), std::move(aIr) });
  }
}