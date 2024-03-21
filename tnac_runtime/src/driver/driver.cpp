#include "driver/driver.hpp"
#include "common/diag.hpp"
#include "output/common.hpp"

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
    set_callbacks();
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

    if (!m_feedback.load_file(m_settings.run_on()))
    {
      return;
    }

    m_tnac.compile();
  }

  void driver::run_interactive() noexcept
  {
    if (!m_settings.interactive())
      return;

    m_feedback.on_command([this](ast::command cmd) noexcept
      { m_repl.on_command(std::move(cmd)); });
    m_repl.declare_commands();
    m_repl.run();
  }


  // Private members (Callbacks)

  void driver::set_callbacks() noexcept
  {
    m_feedback.on_error([this](string_t msg) noexcept
      { on_error("Generic"sv, msg); });
    m_feedback.on_parse_error([this](const ast::error_expr& err) noexcept
      { on_error(err.at().at(), err.message()); });
    m_feedback.on_compile_error([this](src::loc_wrapper&& lok, string_t msg) noexcept
      { on_error(lok, msg); });
  }

  void driver::error_mark() noexcept
  {
    fmt::print(m_state.err(), fmt::clr::Red, " error: "sv);
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

  void driver::on_error(src::loc_wrapper loc, string_t msg) noexcept
  {
    using namespace out;
    m_state.err() << loc << ':';
    post_error(msg);

    auto line = fetch_line(loc);
    if (line.empty())
      return;

    fmt::add_clr(m_state.err(), fmt::clr::White);
    m_state.err() << line << '\n' << std::setw(loc->col() + 2) << "^\n";
    fmt::clear_clr(m_state.err());
  }

}