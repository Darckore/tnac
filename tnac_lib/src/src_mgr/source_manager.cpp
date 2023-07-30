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
    file_t file{ path };
    if (!file)
      return std::unexpected{ err::file_not_found() };

    const auto hash = file.hash();
    auto [iter, success] = m_files.try_emplace(hash, std::move(file));
    if (!success)
      return std::unexpected{ err::duplicate_file() };

    return &iter->second;
  }

}