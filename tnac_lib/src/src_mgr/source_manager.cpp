#include "src_mgr/source_manager.hpp"

namespace tnac
{
  namespace err = src::err;

  // Special members

  source_manager::~source_manager() noexcept = default;

  source_manager::source_manager() noexcept = default;


  // Public members

  source_manager::load_res source_manager::load(path_t path) noexcept
  {
    file_t file{ path, *this };
    if (!file)
      return std::unexpected{ err::file_not_found() };

    const auto hash = file.hash();
    auto emplaceRes = m_files.try_emplace(hash, std::move(file));
    return &emplaceRes.first->second;
  }

  src::loc_wrapper source_manager::register_location(const src::location& loc) noexcept
  {
    return src::loc_wrapper{ m_locations.emplace_front(loc) };
  }

}