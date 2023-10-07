//
// Command line
//

#pragma once

namespace tnac_rt
{
  namespace detail
  {
    template <typename F>
    concept cli_err_handler = std::is_nothrow_invocable_r_v<void, F, tnac::string_t>;
  }

  //
  // Parses the command line and stores the settings
  //
  class cmdline final
  {
  public:
    using name_t  = tnac::string_t;
    using flags_t = unsigned;

    using err_handler_t = std::move_only_function<void(tnac::string_t) noexcept>;

  public:
    CLASS_SPECIALS_NONE(cmdline);

    ~cmdline() noexcept;

    cmdline(int argCount, char** args) noexcept;

  public:
    //
    // Attaches the error handler
    //
    template <detail::cli_err_handler F>
    void on_error(F&& f) noexcept
    {
      m_errHandler = std::forward<F>(f);
    }

    bool interactive() const noexcept;

  private:
    //
    // Reports an error
    // If no error handler is set, does nothing
    //
    void error(tnac::string_t msg) noexcept;

    //
    // Consumes the next cli arg
    //
    void consume(tnac::string_t arg) noexcept;

    //
    // Parses the command line
    //
    void parse(int argCount, char** args) noexcept;

  private:
    err_handler_t m_errHandler{};
    name_t m_inputFile;
    flags_t m_interactive : 1{};
  };
}