//
// A convenient way to init everything the command interpreter needs
//

#pragma once
#include "parser/commands/cmd_interpreter.hpp"

namespace tnac::packages
{
  //
  // A wrapper for all things cmd
  //
  class cmd final
  {
  public:
    using value_type = tnac::cmd::value_type;
    using name_type  = tnac::commands::store::name_type;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(cmd);

    ~cmd() noexcept;

    cmd() noexcept;

  public:
    //
    // Redirects to tnac::cmd::on_command
    //
    void on_command(value_type command) noexcept;

    //
    // Redirects to tnac::commands::store::declare
    //
    template <typename ...Args>
    void declare(name_type name, Args&& ...args) noexcept
    {
      m_cmdStore.declare(name, std::forward<Args>(args)...);
    }

  public:
    //
    // tnac::cmd::on_error
    //
    template <commands::detail::cmd_err_handler F>
    void on_error(F&& handler) noexcept
    {
      m_cmdInterpreter.on_error(std::forward<F>(handler));
    }

  private:
    tnac::commands::store m_cmdStore;
    tnac::cmd m_cmdInterpreter;
  };
}