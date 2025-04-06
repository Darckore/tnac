#include "common/feedback.hpp"

namespace tnac
{
  namespace
  {
    void dummy_err(string_t) noexcept {}
    void dummy_parse_err(const ast::error_expr&) noexcept {}
    void dummy_compile_err(src::loc_wrapper&&, string_t) noexcept {}
    void dummy_cmd(ast::command&&) noexcept {}
    bool dummy_file(fsys::path) noexcept { return {}; }
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
    m_compileWarningHandler{ dummy_compile_err },
    m_compileNoteHandler{ dummy_compile_err },
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

  void feedback::compile_error(src::loc_wrapper&& loc, string_t msg) noexcept
  {
    m_compileErrorHandler(std::move(loc), msg);
  }

  void feedback::compile_warning(src::loc_wrapper&& loc, string_t msg) noexcept
  {
    m_compileWarningHandler(std::move(loc), msg);
  }

  void feedback::compile_note(src::loc_wrapper&& loc, string_t msg) noexcept
  {
    m_compileNoteHandler(std::move(loc), msg);
  }

  void feedback::command(ast::command&& cmd) noexcept
  {
    m_commandHandler(std::move(cmd));
  }

  bool feedback::load_file(fsys::path path) noexcept
  {
    return m_fileLoader(std::move(path));
  }

}