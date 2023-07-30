#include "src_mgr/source_location.hpp"
#include "src_mgr/source_manager.hpp"

namespace tnac::src
{
  // Special members

  location::~location() noexcept = default;

  location::location(path_ref path, source_manager& mgr) noexcept :
    m_path{ &path },
    m_mgr{ &mgr }
  {}


  // Public members

  void location::add_line() noexcept
  {
    ++m_lineNumber;
  }

  void location::add_col() noexcept
  {
    ++m_column;
  }

  location::line_num location::line() const noexcept
  {
    return m_lineNumber;
  }

  location::line_pos location::col() const noexcept
  {
    return m_column;
  }

  source_manager& location::src_mgr() noexcept
  {
    return *m_mgr;
  }

  location::path_ref location::file() const noexcept
  {
    return *m_path;
  }

  location::hash_t location::file_id() const noexcept
  {
    return file::hash(file());
  }

  loc_wrapper location::record() noexcept
  {
    return src_mgr().register_location(*this);
  }
}