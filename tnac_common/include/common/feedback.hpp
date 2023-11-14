//
// Feedback
//

#pragma once

namespace tnac
{
  class token;

  namespace src
  {
    class file;
  }

  namespace ast
  {
    class error_expr;
    class command;
  }

  namespace detail
  {
    template <typename F>
    concept generic_err_handler = std::is_nothrow_invocable_r_v<void, F, string_t>;

    template <typename F>
    concept parse_err_handler = std::is_nothrow_invocable_r_v<void, F, const ast::error_expr&>;

    template <typename F>
    concept compile_err_handler = std::is_nothrow_invocable_r_v<void, F, const token&, string_t>;

    template <typename F>
    concept cmd_handler = std::is_nothrow_invocable_r_v<void, F, ast::command&&>;

    template <typename F>
    concept file_loader = std::is_nothrow_invocable_r_v<src::file*, F, fsys::path>;
  }
}

namespace tnac
{
  //
  // Provides an interface to provide feedback,
  // e.g., report errors
  //
  class feedback final
  {
  public:
    using gerr_handler_t = std::move_only_function<void(string_t) noexcept>;
    using perr_handler_t = std::move_only_function<void(const ast::error_expr&) noexcept>;
    using cerr_handler_t = std::move_only_function<void(const token&, string_t) noexcept>;
    using cmd_handler_t  = std::move_only_function<void(ast::command&&) noexcept>;
    using file_loader_t  = std::move_only_function<src::file*(fsys::path) noexcept>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(feedback);

    ~feedback() noexcept;

    feedback() noexcept;

  public: // Handler setup
    //
    // Sets a handler for a generic error
    //
    template <detail::generic_err_handler F>
    void on_error(F handler) noexcept
    {
      m_genericErrorHandler = std::move(handler);
    }

    //
    // Sets a handler for a parse error
    //
    template <detail::parse_err_handler F>
    void on_parse_error(F handler) noexcept
    {
      m_parseErrorHandler = std::move(handler);
    }

    //
    // Sets a handler for a compile error
    //
    template <detail::compile_err_handler F>
    void on_compile_error(F handler) noexcept
    {
      m_compileErrorHandler = std::move(handler);
    }

    //
    // Sets a handler for a command
    //
    template <detail::cmd_handler F>
    void on_command(F handler) noexcept
    {
      m_commandHandler = std::move(handler);
    }

    //
    // Sets a file loader
    //
    template <detail::file_loader F>
    void on_load_request(F handler) noexcept
    {
      m_fileLoader = std::move(handler);
    }

  public: // Handler invocations
    //
    // Invokes the generic error handler
    //
    void error(string_t msg) noexcept;

    //
    // Invokes the parse error handler
    //
    void parse_error(const ast::error_expr& err) noexcept;

    //
    // Invokes the compile error handler
    //
    void compile_error(const token& tok, string_t msg) noexcept;

    //
    // Invokes the command handler
    //
    void command(ast::command&& cmd) noexcept;

    //
    // Invokes the file loader
    //
    src::file* load_file(fsys::path path) noexcept;

  private:
    gerr_handler_t m_genericErrorHandler;
    perr_handler_t m_parseErrorHandler;
    cerr_handler_t m_compileErrorHandler;
    cmd_handler_t  m_commandHandler;
    file_loader_t  m_fileLoader;
  };
}