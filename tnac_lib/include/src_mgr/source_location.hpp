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
  class location final
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

  private:
    //
    // Records the current location in internal static storage
    //
    static loc_wrapper rec_internal(const location& loc) noexcept;

  public:
    CLASS_SPECIALS_ALL_CUSTOM(location);

    ~location() noexcept;

    location(path_ref path, source_manager& mgr) noexcept;

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
    // Sets line and column numbers to the specified values
    //
    void set(line_num l, line_pos c) noexcept;

    //
    // Decrements column number by the specified amout
    // will not go past 0
    //
    void decr_column_by(line_pos delta) noexcept;

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

  private:
    path_ptr m_path{};
    source_manager* m_mgr{};
    line_num m_lineNumber{};
    line_pos m_column{};
  };


  //
  // Light wrapper for a source location
  //
  class loc_wrapper final
  {
  public:
    CLASS_SPECIALS_NODEFAULT(loc_wrapper);

    ~loc_wrapper() noexcept = default;

    explicit loc_wrapper(location& loc) noexcept :
      m_loc{ &loc }
    {}

  public:
    explicit operator bool() const noexcept
    {
      return !m_loc->is_dummy();
    }

    const location& operator*() const noexcept
    {
      return *m_loc;
    }
    location& operator*() noexcept
    {
      return FROM_CONST(operator*);
    }

    const location* operator->() const noexcept
    {
      return m_loc;
    }
    location* operator->() noexcept
    {
      return FROM_CONST(operator->);
    }

  private:
    location* m_loc{};
  };
}