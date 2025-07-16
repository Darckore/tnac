#include "src_mgr/source_location.hpp"
#include "src_mgr/source_manager.hpp"

// location
namespace tnac::src
{
  // Statics

  location& location::dummy() noexcept
  {
    static location dummyLoc;
    return dummyLoc;
  }

  // Special members

  location::~location() noexcept = default;

  location::location(path_ref path, source_manager& mgr) noexcept :
    location{ &path, &mgr, {}, {} }
  {}

  location::location(path_ptr path, source_manager* mgr, line_num ln, line_pos lp) noexcept :
    m_path{ path },
    m_mgr{ mgr },
    m_lineNumber{ ln },
    m_column{ lp }
  {}

  location::location() noexcept = default;

  // Public members

  bool location::is_dummy() const noexcept
  {
    return !(m_path && m_mgr);
  }

  location location::clone() const noexcept
  {
    return { m_path, m_mgr, m_lineNumber, m_column };
  }

  void location::decr_column_by(line_pos delta) noexcept
  {
    if (m_column < delta)
    {
      m_column = {};
      return;
    }

    m_column -= delta;
  }

  void location::incr_column_by(line_pos delta) noexcept
  {
    m_column += delta;
  }

  void location::add_line() noexcept
  {
    if (!is_dummy())
    {
      auto thisWrapper = loc_wrapper{ *this };
      if (auto srcFile = src_mgr().fetch_file(thisWrapper))
        srcFile->add_line_info(thisWrapper);
    }

    ++m_lineNumber;
    m_column = {};
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
    UTILS_ASSERT(!is_dummy());
    return *m_mgr;
  }

  location::path_ref location::file() const noexcept
  {
    UTILS_ASSERT(!is_dummy());
    return *m_path;
  }

  location::hash_t location::file_id() const noexcept
  {
    if (is_dummy())
      return hash_t{};

    return file::hash(file());
  }

  loc_wrapper location::record() noexcept
  {
    if (is_dummy())
      return loc_wrapper{ dummy() };

    return src_mgr().register_location(*this);
  }

  void location::remove() noexcept
  {
    if (!is_attached() || is_dummy())
      return;

    SELF_DELETE();
  }
}

// loc_wrapper
namespace tnac::src
{
  // Special members

  loc_wrapper::~loc_wrapper() noexcept = default;

  loc_wrapper::loc_wrapper(location& loc) noexcept :
    rc_base{ loc }
  {
  }


  // Public members

  loc_wrapper::operator bool() const noexcept
  {
    auto loc = operator->();
    return !loc->is_dummy();
  }

  const location& loc_wrapper::operator*() const noexcept
  {
    return *operator->();
  }
  location& loc_wrapper::operator*() noexcept
  {
    return FROM_CONST(operator*);
  }
}