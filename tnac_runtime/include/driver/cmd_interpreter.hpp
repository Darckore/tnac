//
// Command interpreter
//

#pragma once
#include "ast/ast_util.hpp"

namespace tnac_rt
{
  namespace detail
  {
    template <typename F>
    concept err_handler = std::is_nothrow_invocable_r_v<void, F, const tnac::token&, tnac::string_t>;
  }

  //
  // Indicates whether or not a command passes verification
  //
  enum class verification : std::uint8_t
  {
    Correct,
    WrongName,
    TooFew,
    TooMany,
    WrongKind
  };

  //
  // Interprets commands and calls the specified callbacks subscribed to them
  //
  class cmd
  {
  public:
    using err_handler_t = std::function<void(const tnac::token&, tnac::string_t)>;
    using value_type = tnac::ast::command;
    using cmd_ptr = const value_type*;
    using cmd_ref = const value_type&;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(cmd);

    ~cmd() noexcept = default;

    cmd() noexcept = default;

  public:
    //
    // Interprets a command
    //
    void on_command(value_type command) noexcept
    {
      utils::unused(command);
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
    void on_error(cmd_ref command) noexcept
    {
      utils::unused(command);
    }

  private:
    err_handler_t m_errHandler{};
  };
}