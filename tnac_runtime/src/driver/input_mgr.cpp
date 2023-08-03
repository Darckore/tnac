#include "driver/input_mgr.hpp"

namespace tnac_rt
{
  // Special members

  input_mgr::~input_mgr() noexcept = default;

  input_mgr::input_mgr(src_mgr& mgr) noexcept :
    m_srcMgr{ mgr }
  {}

  // Public members

  input_mgr::stored_input& input_mgr::input(tnac::buf_t in) noexcept
  {
    loc_t::dummy().set(m_inputIdx, 0);
    auto newItem = m_input.try_emplace(m_inputIdx++, std::move(in));
    return newItem.first->second;
  }

  input_mgr::stored_input* input_mgr::from_file(tnac::string_t fname) noexcept
  {
    auto loadRes = m_srcMgr.load(fname);
    if (!loadRes)
    {
      err() << "Unable to load file '" << fname << "': "
            << loadRes.error().message() << '\n';
      return {};
    }

    auto file = *loadRes;
    if (auto fileCont = file->get_contents(); !fileCont)
    {
      err() << "Unable to read data from file '" << fname << "': "
            << loadRes.error().message() << '\n';
      return {};
    }

    const auto key = file->hash();
    auto newItem = m_files.try_emplace(key, file);
    return &newItem.first->second;
  }

  void input_mgr::on_error(const tnac::token& tok, tnac::string_t msg) noexcept
  {
    auto loc = tok.at();
    print_location(loc);
    print_error();
    err() << msg << '\n';
    print_line(loc);
  }

  void input_mgr::on_parse_error(const tnac::ast::error_expr& error) noexcept
  {
    on_error(error.at(), error.message());
  }

  // Private members

  void input_mgr::print_location(loc_wrapper at) noexcept
  {
    err() << '<';
    if (at)
      err() << at->file().string();
    else
      err() << "REPL";

    err() << ">:" << (at->line() + 1) << ':' << (at->col() + 1);
  }

  void input_mgr::print_line(loc_wrapper at) noexcept
  {
    auto line = get_line(at);
    if (line.empty())
      return;

    err() << line << '\n';
    for (auto idx = at->col(); idx > 0; --idx)
      err() << ' ';
    
    err() << "^\n";
  }

  void input_mgr::print_error() noexcept
  {
#if TNAC_WINDOWS
    err() << "\x1b[31m";
#endif

    err() << " error: ";

#if TNAC_WINDOWS
    err() << "\x1b[39m";
#endif
  }

  tnac::string_t input_mgr::get_line(loc_wrapper at) noexcept
  {
    if (!at)
      return get_repl(at);

    return get_line_from_file(at);
  }

  tnac::string_t input_mgr::get_repl(loc_wrapper at) noexcept
  {
    auto line = m_input.find(at->line());
    if (line == m_input.end())
      return {};

    return line->second.buffer();
  }

  tnac::string_t input_mgr::get_line_from_file(loc_wrapper at) noexcept
  {
    auto key = at->file_id();
    auto file = m_files.find(key);
    if (file == m_files.end())
      return {};

    auto buf = file->second.buffer();
    using line_num = loc_t::line_num;
    const auto targetLine = at->line();
    for (auto idx = line_num{}; auto line : utils::split(buf, "\n"))
    {
      if (idx == targetLine)
        return line;
      ++idx;
    }

    return {};
  }

  out_stream& input_mgr::err() noexcept
  {
    return *m_err;
  }

}