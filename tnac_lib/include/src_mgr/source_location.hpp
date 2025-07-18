//
// Source location
//

#pragma once

namespace tnac
{
  class source_manager;
}

namespace tnac::src
{
  class loc_wrapper;

  //
  // Stores information about locations of entities in source files
  //
  class location final :
    public utils::ilist_node<location>,
    public ref_counted<location>
  {
  public:
    using hash_t   = std::size_t;
    using path_t   = fsys::path;
    using path_ptr = const path_t*;
    using path_ref = const path_t&;

    using line_num = std::uint32_t;
    using line_pos = std::uint32_t;

  public:
    //
    // Retrieves a reference to the static dummy location object
    //
    static location& dummy() noexcept;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(location);

    ~location() noexcept;

    location(path_ref path, source_manager& mgr) noexcept;

    location(path_ptr path, source_manager* mgr, line_num ln, line_pos lp) noexcept;

  private:
    location() noexcept;

  public:
    //
    // Checks whether the specified location is "dummy",
    // that is, it has only line and column numbers, and doesn't
    // refer to any valid path
    //
    bool is_dummy() const noexcept;

    //
    // Clones the location as a stack object
    //
    location clone() const noexcept;

    //
    // Decrements column number by the specified amout
    // will not go past 0
    //
    void decr_column_by(line_pos delta) noexcept;

    //
    // Inrements column number by the specified amout
    //
    void incr_column_by(line_pos delta) noexcept;

    //
    // Increments line number
    //
    void add_line() noexcept;

    //
    // Increments column number
    //
    void add_col() noexcept;

    //
    // Returns line number
    //
    line_num line() const noexcept;

    //
    // Returns position in the current line
    //
    line_pos col() const noexcept;

    //
    // Returns a reference to the source manager
    //
    source_manager& src_mgr() noexcept;

    //
    // Returns the full path to its corresponding file
    //
    path_ref file() const noexcept;

    //
    // Hashes the file path
    //
    hash_t file_id() const noexcept;

    //
    // Records this instance of source location and returns a wrapper to it
    //
    loc_wrapper record() noexcept;

    //
    // Removes the current object from the location list
    //
    void remove() noexcept;

  private:
    path_ptr m_path{};
    source_manager* m_mgr{};
    line_num m_lineNumber{};
    line_pos m_column{};
  };


  //
  // Light wrapper for a source location
  //
  class loc_wrapper final :
    public rc_wrapper<location>
  {
  public:
    using rc_base = rc_wrapper<location>;

  public:
    CLASS_SPECIALS_NODEFAULT(loc_wrapper);

    ~loc_wrapper() noexcept;

    explicit loc_wrapper(location& loc) noexcept;

  public:
    explicit operator bool() const noexcept;

    const location& operator*() const noexcept;
    location& operator*() noexcept;
  };
}