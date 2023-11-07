#include "input/repl.hpp"
#include "driver/state.hpp"
#include "output/printer.hpp"
#include "output/lister.hpp"

namespace tnac::rt
{
  // Special members

  repl::~repl() noexcept = default;

  repl::repl(state& st) noexcept :
    m_state{ &st }
  {}


  // Public members

  void repl::run() noexcept
  {
    buf_t input;
    m_state->start();

    while (m_state->is_running())
    {
      m_state->out() << ">> ";
      std::getline(m_state->in(), input);
      if (utils::ltrim(input).empty())
      {
        m_state->out() << "Enter an expression\n";
        continue;
      }

      auto parseRes = m_state->tnac_core().parse(input);
      if (parseRes)
        m_last = parseRes;

      utils::unused(parseRes);

      input = {};
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

    //core.declare_cmd("vars"sv, params{ String }, size_type{},
    //     [this](auto c) noexcept { print_vars(std::move(c)); });

    //core.declare_cmd("bin"sv, [this](auto) noexcept { set_num_base(2); });
    //core.declare_cmd("oct"sv, [this](auto) noexcept { set_num_base(8); });
    //core.declare_cmd("dec"sv, [this](auto) noexcept { set_num_base(10); });
    //core.declare_cmd("hex"sv, [this](auto) noexcept { set_num_base(16); });

    m_commandsReady = true;
  }

  void repl::on_command(ast::command cmd) noexcept
  {
    m_state->tnac_core().process_cmd(std::move(cmd));
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
    utils::unused(cmd);
  }

  void repl::print_ast(ast::command cmd) noexcept
  {
    utils::unused(cmd);
  }

}