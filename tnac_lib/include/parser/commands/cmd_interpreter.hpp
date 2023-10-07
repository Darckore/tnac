//
// Command interpreter
//

#pragma once
#include "parser/ast/ast_util.hpp"
#include "parser/commands/cmd/commands.hpp"

namespace tnac
{
  //
  // Interprets commands and calls the specified callbacks subscribed to them
  //
  class cmd final
  {
  public:
    using err_handler_t = std::function<void(const token&, string_t)>;
    using value_type    = ast::command;
    using cmd_ptr       = const value_type*;
    using cmd_ref       = const value_type&;
    using cmd_store     = commands::store;
    using descr_ref     = const cmd_store::cmd_descr&;
    using ver_result    = commands::ver_result;

  public:
    CLASS_SPECIALS_NONE(cmd);

    ~cmd() noexcept;

    cmd(cmd_store& store) noexcept;

  public:
    //
    // Interprets a command
    //
    void on_command(value_type command) const noexcept;

    //
    // Sets an error callback
    //
    template <commands::detail::err_handler F>
    void on_error(F&& handler) noexcept
    {
      m_errHandler = std::forward<F>(handler);
    }

  private:
    //
    // Verifies a command
    //
    ver_result verify(cmd_ref command, descr_ref base) const noexcept;

    //
    // Produces an error
    //
    void on_error(cmd_ref command, const ver_result& reason) const noexcept;

  private:
    err_handler_t m_errHandler{};
    cmd_store& m_cmdStore;
  };
}