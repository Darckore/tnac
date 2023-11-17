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

namespace tnac::ast
{
  class module_def;
}

namespace tnac::src
{
  //
  // Represents a source file
  //
  class file final
  {
  private:
    struct line
    {
      using size_type = string_t::size_type;
      size_type beg{};
      size_type end{};
    };

  public:
    using path_t     = fsys::path;
    using hash_t     = std::size_t;
    using contents_t = file_data;

    using line_num_t = location::line_num;
    using lines_t    = std::vector<line>;

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

    //
    // Retrieves a line by number
    //
    string_t fetch_line(line_num_t ln) const noexcept;

    //
    // Adds line information for a new line
    //
    void add_line_info(src::loc_wrapper loc) noexcept;

    //
    // Attaches AST of the resulting module
    //
    void attach_ast(ast::module_def& mod) noexcept;

    //
    // Returns the previously cached AST
    // 
    // const version
    //
    const ast::module_def* parsed_ast() const noexcept;

    //
    // Returns the previously cached AST
    //
    ast::module_def* parsed_ast() noexcept;

  private:
    //
    // Reads the entire file into the internal buffer
    //
    bool read() noexcept;

  private:
    path_t m_path{};
    buf_t m_buffer;
    source_manager* m_mgr{};
    ast::module_def* m_parsed{};
    lines_t m_lines;
  };
}