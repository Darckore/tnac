#include "common/feedback.hpp"

namespace tnac
{
  namespace
  {
    void dummy_err(string_t) noexcept {}
    void dummy_parse_err(const ast::error_expr&) noexcept {}
    void dummy_compile_err(const token&, string_t) noexcept {}
    void dummy_cmd(ast::command&&) noexcept {}
    src::file* dummy_file(fsys::path) noexcept { return {}; }
  }
}

namespace tnac
{
  // Special members

  feedback::~feedback() noexcept = default;

  feedback::feedback() noexcept :
    m_genericErrorHandler{ dummy_err },
    m_parseErrorHandler{ dummy_parse_err },
    m_compileErrorHandler{ dummy_compile_err },
    m_commandHandler{ dummy_cmd },
    m_fileLoader{ dummy_file }
  {}


  // Public members

  void feedback::error(string_t msg) noexcept
  {
    m_genericErrorHandler(msg);
  }

  void feedback::parse_error(const ast::error_expr& err) noexcept
  {
    m_parseErrorHandler(err);
  }

  void feedback::compile_error(const token& tok, string_t msg) noexcept
  {
    m_compileErrorHandler(tok, msg);
  }

  void feedback::command(ast::command&& cmd) noexcept
  {
    m_commandHandler(std::move(cmd));
  }

  src::file* feedback::load_file(fsys::path path) noexcept
  {
    return m_fileLoader(std::move(path));
  }

}