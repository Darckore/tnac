#include "src_mgr/source_manager.hpp"

namespace tnac
{
  namespace err = src::err;


  // Statics

  source_manager::path_t source_manager::canonise(const path_t& src) noexcept
  {
    std::error_code errc;
    auto res = fsys::absolute(src, errc);
    res = fsys::weakly_canonical(res, errc);
    if (errc) res.clear();
    return res;
  }


  // Special members

  source_manager::~source_manager() noexcept = default;

  source_manager::source_manager() noexcept = default;


  // Public members

  source_manager::load_res source_manager::load(path_t path) noexcept
  {
    auto loadPath = canonise(path);
    if (loadPath.empty() || !file_t::exists(loadPath))
      return std::unexpected{ load_err{ err::file_not_found(), std::move(loadPath) } };

    const auto hash = file_t::hash(loadPath);
    auto emplaceRes = m_files.try_emplace(hash, file_t{ std::move(loadPath), *this });
    return &emplaceRes.first->second;
  }

  src::loc_wrapper source_manager::register_location(const src::location& loc) noexcept
  {
    return src::loc_wrapper{ m_locations.emplace_front(loc) };
  }

  const source_manager::file_t* source_manager::fetch_file(src::loc_wrapper loc) const noexcept
  {
    auto fileIt = m_files.find(loc->file_id());
    return fileIt != m_files.end() ? &fileIt->second : nullptr;
  }

  source_manager::file_t* source_manager::fetch_file(src::loc_wrapper loc) noexcept
  {
    return FROM_CONST(fetch_file, loc);
  }

  string_t source_manager::fetch_line(src::loc_wrapper loc) const noexcept
  {
    auto srcFile = fetch_file(loc);
    if (!srcFile)
      return {};

    return srcFile->fetch_line(loc->line());
  }

}