#include "input/repl.hpp"
#include "output/printer.hpp"
#include "output/lister.hpp"
#include "common/feedback.hpp"
#include "common/diag.hpp"

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

    while (m_state->is_running())
    {
      auto input = consume_input();
      if (input.empty())
        continue;

      auto parseRes = m_state->tnac_core().parse(input, m_loc);
      if (parseRes && parseRes != m_state->tnac_core().get_ast())
        m_last = parseRes;

      utils::unused(parseRes);
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

    core.declare_cmd("vars"sv, params{ String }, size_type{},
         [this](auto c) noexcept { print_vars(std::move(c)); });

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

  string_t repl::consume_input() noexcept
  {
    buf_t input;

    m_state->out() << ">> ";
    std::getline(m_state->in(), input);
    if (utils::ltrim(input).empty())
    {
      m_state->out() << "Enter an expression\n";
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
      m_feedback->compile_error(path, diag::file_write_failure(fileName, "not accessible"sv));

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
    m_state->stop();
  }

  void repl::print_result(ast::command cmd) noexcept
  {
    utils::unused(cmd);
  }

  void repl::list_code(ast::command cmd) noexcept
  {
    print_cmd(cmd, [this]
      {
        out::lister ls;
        if (m_state->in_stdout()) ls.enable_styles();
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

        m_feedback->compile_error(second, diag::wrong_cmd_arg(curIdx, second.value()));
        return core.get_ast();
      };

    auto ast = toPrint(cmd);
    print_cmd(cmd, [this, ast]
      {
        out::ast_printer pr;
        if (m_state->in_stdout()) pr.enable_styles();
        pr(ast, m_state->out());
      });
  }

  void repl::print_vars(ast::command cmd) noexcept
  {
    print_cmd(cmd, [this]
      {
        auto varCollection = m_state->tnac_core().variables();
        for (auto it = varCollection.begin(); it != varCollection.end(); ++it)
        {
          auto scope = it.scope();
          m_state->out() << scope << ":\n";
          for (auto var : *it)
          {
            m_state->out() << ' ' << var->name() << '\n';
          }
        }
      });
  }
}