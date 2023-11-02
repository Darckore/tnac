//
// Command line
//

#pragma once

namespace tnac
{
  class feedback;
}

namespace tnac::rt
{
  //
  // Parses the command line and stores the settings
  //
  class cmdline final
  {
  public:
    using name_t  = string_t;
    using flags_t = unsigned;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(cmdline);

    ~cmdline() noexcept;

    cmdline() noexcept;

  public:
    //
    // Attaches the feedback object
    //
    void attach_feedback(feedback& fb) noexcept;

    //
    // Parses the command line
    //
    void parse(int argCount, char** args) noexcept;

    //
    // Returns true if the target file is specified
    //
    bool has_input_file() const noexcept;

    //
    // Returns the input file name
    //
    name_t run_on() const noexcept;

    //
    // Reports the state of the -i flag
    //
    bool interactive() const noexcept;

  private:
    //
    // Reports an error
    // If no error handler is set, does nothing
    //
    void error(string_t msg) noexcept;

    //
    // Consumes the next cli arg
    //
    void consume(string_t arg) noexcept;

  private:
    feedback* m_feedback{};

    struct state
    {
      name_t m_inputFile;
      flags_t m_interactive : 1{};
    };

    state m_state{};
  };
}