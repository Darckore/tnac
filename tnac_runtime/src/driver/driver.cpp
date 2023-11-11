#include "driver/driver.hpp"
#include "common/diag.hpp"

namespace tnac::rt
{
  // Special members

  driver::~driver() noexcept = default;

  driver::driver(int argCount, char** args) noexcept :
    m_settings{ m_feedback },
    m_tnac{ m_feedback },
    m_state{ m_tnac },
    m_repl{ m_state, m_feedback }
  {
    m_feedback.on_error([this](string_t msg) noexcept { on_error("Command line"sv, msg); });
    m_settings.parse(argCount, args);
    run();
    run_interactive();
  }


  // Private members

  void driver::run() noexcept
  {
    if (!m_settings.has_input_file())
    {
      return;
    }

    set_file_callbacks();
    auto loadResult = m_tnac.load(m_settings.run_on());
    if (!loadResult)
    {
      auto&& err = loadResult.error();
      m_feedback.error(diag::file_load_failure(err.second, err.first.message()));
      return;
    }

    auto&& inputFile = *loadResult;
    fsys::current_path(inputFile->directory());
    auto parseRes = m_tnac.parse(*inputFile);
    utils::unused(parseRes);

    // todo: compile
  }

  void driver::run_interactive() noexcept
  {
    if (!m_settings.interactive())
      return;

    set_repl_callbacks();
    m_repl.declare_commands();
    m_repl.run();
  }


  // Private members (Callbacks)

  void driver::set_file_callbacks() noexcept
  {
    m_feedback.on_error([this](string_t msg) noexcept
      { on_error("Input"sv, msg); });
    m_feedback.on_parse_error([this](const ast::error_expr& err) noexcept
      { on_error(err.at(), err.message()); });
    m_feedback.on_compile_error([this](const token& tok, string_t msg) noexcept
      { on_error(tok, msg); });
  }

  void driver::set_repl_callbacks() noexcept
  {
    m_feedback.on_command([this](ast::command cmd) noexcept
      { m_repl.on_command(std::move(cmd)); });
  }

  void driver::error_mark() noexcept
  {
    fmt::print(m_state.err(), fmt::clr::BoldRed, " error: "sv);
  }

  void driver::post_error(string_t msg) noexcept
  {
    error_mark();
    m_state.err() << msg << '\n';
  }

  string_t driver::fetch_line(src::loc_wrapper at) noexcept
  {
    if (auto replLine = m_repl.fetch_line(at))
      return *replLine;

    return m_tnac.fetch_line(at);
  }

  void driver::on_error(string_t prefix, string_t msg) noexcept
  {
    m_state.err() << '<' << prefix << '>';
    post_error(msg);
  }

  void driver::on_error(const token& tok, string_t msg) noexcept
  {
    auto loc = tok.at();
    m_state.err() << '<';
    if (loc)
      m_state.err() << loc->file().string();
    else
      m_state.err() << "Unknown"sv;

    m_state.err() << ">:" << (loc->line() + 1) << ':' << (loc->col() + 1) << ':';
    post_error(msg);

    auto line = fetch_line(loc);
    if (line.empty())
      return;

    fmt::add_clr(m_state.err(), fmt::clr::White);
    m_state.err() << line << '\n' << std::setw(loc->col() + 2) << "^\n";
    fmt::clear_clr(m_state.err());
  }

}