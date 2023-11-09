#include "src_mgr/source_file.hpp"

namespace tnac::src
{
  // Statics

  file::hash_t file::hash(const path_t& path) noexcept
  {
    return fsys::hash_value(path);
  }

  bool file::exists(const path_t & path) noexcept
  {
    return fsys::exists(path);
  }


  // Special members

  file::~file() noexcept = default;

  file::file(path_t path, source_manager& mgr) noexcept :
    m_path{ std::move(path) },
    m_mgr{ &mgr }
  {}

  file::operator bool() const noexcept
  {
    return exists(m_path);
  }


  // Public members

  file::contents_t file::get_contents() noexcept
  {
    if (!m_buffer.empty())
      return m_buffer;

    if(!read())
    {
      return std::unexpected{ err::file_not_found() };
    }

    return m_buffer;
  }

  const file::path_t& file::path() const noexcept
  {
    return m_path;
  }

  buf_t file::extract_name() const noexcept
  {
    return path().stem().string();
  }

  file::path_t file::directory() const noexcept
  {
    return path().parent_path();
  }

  file::hash_t file::hash() const noexcept
  {
    return hash(path());
  }

  source_manager& file::src_mgr() const noexcept
  {
    return *m_mgr;
  }

  location file::make_location() const noexcept
  {
    return { path(), src_mgr() };
  }

  string_t file::fetch_line(line_num_t ln) const noexcept
  {
    using size_type = line::size_type;
    const auto lineNum = static_cast<size_type>(ln);

    line info{};
    if (const auto sz = m_lines.size(); lineNum >= sz) // Possibly, currently parsed line
    {
      const auto last = lineNum ? lineNum - 1 : size_type{};
      if (last < sz)
        info = m_lines[last];
      else if (!last)
        info = {};
      else
        return {};

      if (info.end > m_buffer.size())
        return {};

      auto newlinePos = m_buffer.find('\n', info.end);
      if (newlinePos == buf_t::npos)
        newlinePos = m_buffer.size();

      info.beg = info.end;
      info.end = newlinePos;
    }
    else
    {
      info = m_lines[lineNum];
    }

    if (info.end > m_buffer.size())
      return {};

    auto bufIt = m_buffer.begin();
    return utils::rtrim(string_t{ std::next(bufIt, info.beg), std::next(bufIt, info.end) });
  }

  void file::add_line_info(src::loc_wrapper loc) noexcept
  {
    using size_type = line::size_type;
    const auto lineNum = static_cast<size_type>(loc->line());
    if (lineNum != m_lines.size())
    {
      UTILS_ASSERT(false);
      return;
    }

    const auto beg = lineNum ? m_lines[lineNum - 1].end : size_type{};
    const auto end = static_cast<size_type>(loc->col() + 1) + beg;
    m_lines.emplace_back(beg, end);
  }


  // Private members

  bool file::read() noexcept
  {
    m_buffer.clear();
    if (!exists(m_path))
      return false;

    std::ifstream in{ m_path.string() };
    if (!in)
      return false;

    in.seekg(0, std::ios::end);
    m_buffer.reserve(in.tellg());
    in.seekg(0, std::ios::beg);

    using it = std::istreambuf_iterator<tnac::buf_t::value_type>;
    m_buffer.assign(it{ in }, it{});
    return true;
  }

}