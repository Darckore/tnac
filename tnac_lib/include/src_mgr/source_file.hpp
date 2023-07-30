//
// Source file
//

#pragma once

namespace tnac::src
{
  //
  // Represents a source file
  //
  class file final
  {
  public:
    using path_t     = fsys::path;
    using contents_t = std::expected<string_t, std::error_code>;

  private:
    //
    // Converts the path to its canonical form
    //
    static path_t canonise(path_t src) noexcept;

  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(file);

    ~file() noexcept;

    explicit file(path_t path) noexcept;

    //
    // Checks whether the physical file exists at the specified path
    //
    explicit operator bool() const noexcept;

  public:
    //
    // Returns the contents of the underlying buffer
    //
    contents_t get_contents() noexcept;

    
  private:
    //
    // Allows checking whether the file exists
    //
    bool exists() const noexcept;

    //
    // Reads the entire file into the internal buffer
    //
    bool read() noexcept;

  private:
    path_t m_path{};
    buf_t m_buffer;
  };
}