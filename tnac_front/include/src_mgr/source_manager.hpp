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
    using loc_store  = std::forward_list<src::location>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(source_manager);

    ~source_manager() noexcept;
    
    source_manager() noexcept;

  private:
    //
    // Converts the path to its canonical form
    //
    static path_t canonise(const path_t& src) noexcept;

  public:
    //
    // Loads a file
    //
    load_res load(path_t path) noexcept;

    //
    // Registers a source location object and returns a wrapper to it
    //
    src::loc_wrapper register_location(const src::location& loc) noexcept;

    //
    // Retrieves the source file by location
    // 
    // const version
    //
    const file_t* fetch_file(src::loc_wrapper loc) const noexcept;

    //
    // Retrieves the source file by location
    // 
    file_t* fetch_file(src::loc_wrapper loc) noexcept;

    //
    // Retrieves the source line by location
    //
    string_t fetch_line(src::loc_wrapper loc) const noexcept;

  private:
    file_store m_files;
    loc_store  m_locations;
  };
}