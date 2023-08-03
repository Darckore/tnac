#include "driver/input_mgr.hpp"

namespace tnac_rt::colours
{
  namespace
  {
    enum class clr : std::uint8_t
    {
      Default,
      Red,
      Yellow,
      Green,
      Blue,
      White
    };

#if TNAC_WINDOWS
    //
    // https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
    //

    void with_colour(std::ostream& out, clr c) noexcept
    {
      using enum clr;
      switch (c)
      {
      case Red:    out << "\x1b[31m"; break;
      case Green:  out << "\x1b[32m"; break;
      case Yellow: out << "\x1b[33m"; break;
      case Blue:   out << "\x1b[34m"; break;
      case White:  out << "\x1b[37m"; break;
      default:                        break;
      }
    }

    void add_clr(std::ostream& out, clr c, bool bold) noexcept
    {
      with_colour(out, c);
      if (bold)
      {
        out << "\x1b[1m";
      }
    }

    void clear_clr(std::ostream& out) noexcept
    {
      out << "\x1b[m";
    }
#else
    void with_colour(std::ostream& out, clr c) noexcept
    {
      utils::unused(out, c);
    }

    void add_clr(std::ostream& out, clr c, bool bold) noexcept
    {
      utils::unused(out, c, bold);
    }

    void clear_clr(std::ostream& out) noexcept
    {
      utils::unused(out);
    }
#endif
  }
}

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
      print_error();
      err() << "Unable to load file '" << fname << "': "
            << loadRes.error().message() << '\n';
      return {};
    }

    auto file = *loadRes;
    if (auto fileCont = file->get_contents(); !fileCont)
    {
      print_error();
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

    colours::add_clr(err(), colours::clr::Default, true);
    err() << msg << '\n';
    colours::clear_clr(err());
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

    colours::add_clr(err(), colours::clr::White, false);
    err() << line << '\n';
    for (auto idx = at->col(); idx > 0; --idx)
      err() << ' ';
    
    err() << "^\n";
    colours::clear_clr(err());
  }

  void input_mgr::print_error() noexcept
  {
    colours::add_clr(err(), colours::clr::Red, true);
    err() << " error: ";
    colours::clear_clr(err());
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