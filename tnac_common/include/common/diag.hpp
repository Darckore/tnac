//
// Diagnostics
//

#pragma once

namespace tnac
{
  class token;

  namespace ast
  {
    class error_expr;
    class command;
  }

  namespace detail
  {
    template <typename F>
    concept parse_err_handler = std::is_nothrow_invocable_r_v<void, F, const ast::error_expr&>;

    template <typename F>
    concept sema_err_handler = std::is_nothrow_invocable_r_v<void, F, const token&, string_t>;

    template <typename F>
    concept cmd_handler = std::is_nothrow_invocable_r_v<void, F, ast::command>;
  }
}

namespace tnac
{
  //
  // Provides an interface to report errors
  //
  class diag final
  {
  public:
    using perr_handler_t = std::move_only_function<void(const ast::error_expr&) noexcept>;
    using serr_handler_t = std::move_only_function<void(const token&, string_t) noexcept>;
    using cmd_handler_t  = std::move_only_function<void(ast::command) noexcept>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(diag);

    ~diag() noexcept;

    diag() noexcept;

  private:

  };
}