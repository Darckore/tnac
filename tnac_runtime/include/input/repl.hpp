//
// REPL
//

#pragma once
#include "src_mgr/source_manager.hpp"
#include "driver/state.hpp"
#include "parser/ast/ast.hpp"

namespace tnac::semantics
{
  class scope;
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
    using line_opt  = std::optional<string_t>;

  public:
    CLASS_SPECIALS_NONE(repl);

    ~repl() noexcept;

    repl(state& st, feedback& fb) noexcept;

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

    //
    // Retrieves a line by location if possible
    //
    line_opt fetch_line(src::loc_wrapper loc) noexcept;

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
    // Generalised print function used in different command handlers
    //
    template <std::invocable<> F>
    void print_cmd(const ast::command& cmd, F&& printFunc) noexcept
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

    //
    // Generalised function for printing vars and funcs
    //
    template <semantics::detail::sym S>
    void print_sym_collection(semantics::sym_container<S> collection) noexcept
    {
      auto styles = m_state->in_stdout();
      for (auto it = collection.begin(); it != collection.end(); ++it)
      {
        m_state->out() << "In scope '";
        print_scope(it.scope(), styles);
        m_state->out() << "':\n";
        for (auto sym : *it)
        {
          m_state->out() << ' ';
          print_sym(*sym, styles);
          m_state->out() << '\n';
        }
      }
    }

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

    //
    // #vars <'path'>
    //
    void print_vars(ast::command cmd) noexcept;

    //
    // #funcs <'path'>
    //
    void print_funcs(ast::command cmd) noexcept;

    //
    // Prints info on the scope of a symbol
    //
    void print_scope(const semantics::scope* scope, bool styles) noexcept;

    //
    // Prints variable information
    //
    void print_var(const semantics::variable& var, bool styles) noexcept;

    //
    // Prints parameter information
    //
    void print_param(const semantics::parameter& par, bool styles) noexcept;

    //
    // Prints function information
    //
    void print_func(const semantics::function& func, bool styles) noexcept;

    //
    // Prints symbol information
    //
    void print_sym(const semantics::symbol& sym, bool styles) noexcept;

  private:
    source_manager m_srcMgr;
    source_manager::path_t m_fake{ "REPL" };
    src::location m_loc;
    input_map m_inputs;

    state* m_state{};
    feedback* m_feedback{};
    ast::node* m_last{};
    bool m_commandsReady{};
  };
}