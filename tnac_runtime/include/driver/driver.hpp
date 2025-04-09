//
// Driver
//

#pragma once
#include "driver/state.hpp"
#include "input/cmdline.hpp"
#include "input/repl.hpp"
#include "core/tnac.hpp"

namespace tnac::rt
{
  //
  // System driver
  // Reads input and evaluates expressions
  //
  class driver final
  {
  public:
    CLASS_SPECIALS_NONE(driver);

    ~driver() noexcept;

    driver(int argCount, char** args) noexcept;

  private:
    //
    // Runs the driver with the provided input
    //
    void run() noexcept;

    //
    // Runs the interactive mode, if specified
    //
    void run_interactive() noexcept;

  private: // Callbacks
    //
    // Inits callbacks used when a file is processed
    //
    void set_callbacks() noexcept;

    //
    // Prints the error mark
    //
    void error_mark() noexcept;

    //
    // Prints the warning mark
    //
    void warning_mark() noexcept;

    //
    // Prints the note mark
    //
    void note_mark() noexcept;

    //
    // Posts an error message
    //
    void post_error(string_t msg) noexcept;

    //
    // Posts a warning message
    //
    void post_warning(string_t msg) noexcept;

    //
    // Posts a note message
    //
    void post_note(string_t msg) noexcept;

    //
    // Retrieves a code line by location
    //
    string_t fetch_line(src::loc_wrapper at) noexcept;

    //
    // Retrieves a line by location and posts it to the given stream
    //
    void post_line(out_stream& stream, src::loc_wrapper at) noexcept;

    //
    // Posts a generic error
    //
    void on_error(string_t prefix, string_t msg) noexcept;

    //
    // Posts a compile or parse error
    //
    void on_error(src::loc_wrapper loc, string_t msg) noexcept;

    //
    // Posts a warning
    //
    void on_warning(src::loc_wrapper loc, string_t msg) noexcept;

    //
    // Posts a note
    //
    void on_note(src::loc_wrapper loc, string_t msg) noexcept;

  private:
    feedback m_feedback;
    cmdline m_settings;
    core m_tnac;
    state m_state;
    repl m_repl;
    bool m_parseOnly{};
  };
}