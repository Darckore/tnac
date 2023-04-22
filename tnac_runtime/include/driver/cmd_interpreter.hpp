//
// Command interpreter
//

#pragma once
#include "ast/ast_util.hpp"
#include "driver/cmd/commands.hpp"

namespace tnac_rt
{
  namespace detail
  {
    template <typename F>
    concept err_handler = std::is_nothrow_invocable_r_v<void, F, const tnac::token&, tnac::string_t>;
  }

  //
  // Interprets commands and calls the specified callbacks subscribed to them
  //
  class cmd
  {
  public:
    using err_handler_t = std::function<void(const tnac::token&, tnac::string_t)>;
    using value_type    = tnac::ast::command;
    using cmd_ptr       = const value_type*;
    using cmd_ref       = const value_type&;
    using cmd_store     = commands::store;

  public:
    CLASS_SPECIALS_NONE(cmd);

    ~cmd() noexcept = default;

    cmd(cmd_store& store) noexcept :
      m_cmdStore{ store }
    {}

  public:
    //
    // Interprets a command
    //
    void on_command(value_type command) noexcept
    {
      using enum commands::verification;
      auto descr = m_cmdStore.find(command.name());
      if (!descr)
      {
        on_error(command, WrongName);
        return;
      }

      // todo: verification
      descr->handler()(std::move(command));
    }

    //
    // Sets an error callback
    //
    template <detail::err_handler F>
    void on_error(F&& handler) noexcept
    {
      m_errHandler = std::forward<F>(handler);
    }

  private:
    //
    // Produces an error
    //
    void on_error(cmd_ref command, commands::verification reason) noexcept
    {
      if (!m_errHandler)
        return;

      utils::unused(command, reason);
    }

  private:
    err_handler_t m_errHandler{};
    cmd_store& m_cmdStore;
  };
}