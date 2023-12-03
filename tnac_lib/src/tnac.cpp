#include "core/tnac.hpp"
#include "common/diag.hpp"

namespace tnac
{
  // Special members

  core::~core() noexcept = default;

  core::core(feedback& fb) noexcept :
    m_feedback{ &fb },
    m_parser{ m_astBuilder, m_sema, &fb },
    m_cmdInterpreter{ m_cmdStore, fb },
    m_compiler{ m_sema, &fb }
  {
    m_feedback->on_load_request([this](fname_t path) noexcept { return process_file(std::move(path)); });
  }


  // Public members

  src::file* core::load_source(fname_t fname) noexcept
  {
    auto loadRes = m_srcMgr.load(std::move(fname));
    if (!loadRes)
    {
      auto&& err = loadRes.error();
      m_feedback->error(diag::file_load_failure(err.second, err.first.message()));
      return {};
    }

    return *loadRes;
  }

  bool core::process_file(fname_t fname) noexcept
  {
    auto loadRes = m_srcMgr.load(std::move(fname));
    if (!loadRes)
    {
      fname = std::move(loadRes.error().second);
      fname.replace_extension("tnac");
      loadRes = m_srcMgr.load(std::move(fname));
    }

    if (loadRes)
    {
      auto file = *loadRes;
      parse(*file);
      return true;
    }

    auto&& err = loadRes.error();
    m_feedback->error(diag::file_load_failure(err.second, err.first.message()));
    return false;
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
    if (!get_ast())
      m_parser(file);
    else
      m_parser.branch()(file);

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