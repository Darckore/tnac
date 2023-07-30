#include "src_mgr/source_file.hpp"

namespace tnac::src
{
  // Statics

  file::path_t file::canonise(path_t src) noexcept
  {
    std::error_code errc;
    src = fsys::weakly_canonical(src, errc);
    if (errc) src.clear();
    return src;
  }


  // Special members

  file::~file() noexcept = default;

  file::file(path_t path) noexcept :
    m_path{ canonise(std::move(path)) }
  {}

  file::operator bool() const noexcept
  {
    return exists();
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

  file::hash_t file::hash() const noexcept
  {
    return fsys::hash_value(path());
  }


  // Private members

  bool file::exists() const noexcept
  {
    return fsys::exists(m_path);
  }

  bool file::read() noexcept
  {
    m_buffer.clear();
    if (!exists())
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