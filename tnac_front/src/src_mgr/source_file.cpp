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