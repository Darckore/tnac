//
// Source file
//

#pragma once
#include "src_mgr/source_location.hpp"

namespace tnac::src::err
{
  inline auto file_not_found() noexcept
  {
    return std::make_error_code(std::errc::no_such_file_or_directory);
  }
}

namespace tnac
{
  class source_manager;
}

namespace tnac::src
{
  //
  // Represents a source file
  //
  class file final
  {
  public:
    using path_t     = fsys::path;
    using hash_t     = std::size_t;
    using contents_t = std::expected<string_t, std::error_code>;

  public:
    //
    // Hashes a path
    //
    static hash_t hash(const path_t& path) noexcept;

    //
    // Checks whether the file at the specified path exists
    //
    static bool exists(const path_t& path) noexcept;

  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(file);

    ~file() noexcept;

  private:
    friend class source_manager;

    file(path_t path, source_manager& mgr) noexcept;

    //
    // Checks whether the physical file exists at the specified path
    //
    explicit operator bool() const noexcept;

  public:
    //
    // Returns the contents of the underlying buffer
    //
    contents_t get_contents() noexcept;

    //
    // Returns the full path
    //
    const path_t& path() const noexcept;

    //
    // Returns file name without extension
    //
    buf_t extract_name() const noexcept;

    //
    // Returns the path to the file's directory
    //
    path_t directory() const noexcept;

    //
    // Returns the hash value for the underlying path
    //
    hash_t hash() const noexcept;

    //
    // Returns the associated source manager
    //
    source_manager& src_mgr() const noexcept;

    //
    // Creates an associated source location
    //
    location make_location() const noexcept;

  private:
    //
    // Reads the entire file into the internal buffer
    //
    bool read() noexcept;

  private:
    path_t m_path{};
    buf_t m_buffer;
    source_manager* m_mgr{};
  };
}