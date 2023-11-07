//
// REPL
//

#pragma once

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
    state* m_state{};
    ast::node* m_last{};
    bool m_commandsReady{};
  };
}