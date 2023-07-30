//
// Source manager
//

#pragma once
#include "src_mgr/source_file.hpp"

namespace tnac
{
  //
  // Loads and maintains source files,
  // and tracks source location info
  //
  class source_manager final
  {
  public:
    using file_t = src::file;
    using path_t = file_t::path_t;
    using hash_t = file_t::hash_t;

    using load_res   = std::expected<file_t*, std::error_code>;
    using file_store = std::unordered_map<hash_t, file_t>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(source_manager);

    ~source_manager() noexcept;
    
    source_manager() noexcept;

  public:
    //
    // Loads a file
    //
    load_res load(path_t path) noexcept;

    //
    // Registers a source location object and returns a wrapper to it
    //
    src::loc_wrapper register_location(src::location& loc) noexcept;

  private:
    file_store m_files;
  };
}