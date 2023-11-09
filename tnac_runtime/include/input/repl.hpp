//
// REPL
//

#pragma once
#include "src_mgr/source_manager.hpp"

namespace tnac
{
  class token;
}

namespace tnac::ast
{
  class node;
  class command;
}

namespace tnac::rt
{
  class state;

  //
  // REPL shell
  // Provides interactive input
  //
  class repl final
  {
  public:
    using input_map = std::unordered_map<std::uint32_t, buf_t>;

  public:
    CLASS_SPECIALS_NONE(repl);

    ~repl() noexcept;

    explicit repl(state& st) noexcept;

  public:
    //
    // Runs the REPL
    //
    void run() noexcept;

    //
    // Declares commands REPL understands
    //
    void declare_commands() noexcept;

    //
    // Processes a command
    //
    void on_command(ast::command cmd) noexcept;

  private:
    //
    // Interns an input string and returns a view into it
    //
    string_t consume_input() noexcept;

    //
    // Attempts to redirect output to the specified file
    //
    bool try_redirect_output(const token& path) noexcept;

    //
    // Resets the output stream
    //
    void end_redirect() noexcept;

  private: // Command handlers
    //
    // #exit
    //
    void on_exit() noexcept;

    //
    // #result
    //
    void print_result(ast::command cmd) noexcept;

    //
    // #list <'path'>
    //
    void list_code(ast::command cmd) noexcept;

    //
    // #ast <'path'> <current>
    //
    void print_ast(ast::command cmd) noexcept;

  private:
    source_manager m_srcMgr;
    source_manager::path_t m_fake{ "REPL" };
    src::location m_loc;
    input_map m_inputs;

    state* m_state{};
    ast::node* m_last{};
    bool m_commandsReady{};
  };
}