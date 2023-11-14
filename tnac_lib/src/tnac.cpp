#include "core/tnac.hpp"

namespace tnac
{
  // Special members

  core::~core() noexcept = default;

  core::core(feedback& fb) noexcept :
    m_feedback{ &fb },
    m_parser{ m_astBuilder, m_sema, &fb },
    m_cmdInterpreter{ m_cmdStore, fb }
  {}


  // Public members

  core::load_res core::load(fname_t fname) noexcept
  {
    return m_srcMgr.load(std::move(fname));
  }

  ast::node* core::parse(string_t input) noexcept
  {
    return m_parser(input);
  }

  ast::node* core::parse(string_t input, src::location& loc) noexcept
  {
    return m_parser(input, loc);
  }

  ast::node* core::parse(src::file& file) noexcept
  {
    m_parser(file);
    return get_ast();
  }

  ast::node* core::get_ast() noexcept
  {
    return m_parser.root();
  }

  void core::process_cmd(ast::command cmd) noexcept
  {
    m_cmdInterpreter.on_command(std::move(cmd));
  }

  string_t core::fetch_line(src::loc_wrapper loc) const noexcept
  {
    return m_srcMgr.fetch_line(loc);
  }

}