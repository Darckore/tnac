//
// Driver
//

#pragma once
#include "packages/parser_pkg.hpp"
#include "packages/evaluator_pkg.hpp"

#include "driver/source_manager.hpp"
#include "commands/cmd_interpreter.hpp"

namespace tnac_rt
{
  //
  // System driver
  // Reads input and evaluates expressions
  //
  class driver final
  {
  public:
    //
    // Sets the output stream for the driver
    //
    friend out_stream& operator<<(out_stream& stream, driver& drv) noexcept;

    //
    // Sets the input stream for the driver
    //
    friend in_stream& operator>>(in_stream& stream, driver& drv) noexcept;

  public:
    using variable_ptr  = const tnac::semantics::variable*;
    using variable_ref  = const tnac::semantics::variable&;
    using var_collection = std::vector<variable_ptr>;

    using parser      = tnac::packages::parser;
    using eval        = tnac::packages::evaluator;

    using command     = tnac::ast::command;
    using size_type   = command::size_type;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(driver);

    driver(int argCount, char** args) noexcept;

  protected:
    //
    // Sets the stream input will be read from
    //
    driver& set_istream(in_stream& stream) noexcept;

    //
    // Sets the stream for output
    //
    driver& set_ostream(out_stream& stream) noexcept;

  private:
    //
    // Inits error handlers
    //
    void init_handlers() noexcept;

    //
    // Parses the command line and launched the driver in the selected the run mode
    //
    void run(int argCount, char** args) noexcept;

    //
    // Reads input from file, parses it, and prints the result
    //
    void run(tnac::string_t fileName) noexcept;

    //
    // Runs in interactive mode until the #exit command is issued
    //
    void run_interactive() noexcept;

  private: // Command handlers
    //
    // #exit
    //
    void on_exit() noexcept;

    //
    // Sets the base for ints
    // The setting persists until a new base is set
    //
    void set_num_base(int base) noexcept;

    //
    // Helper intended to be used by the handler and elsewhere
    //
    void print_result() noexcept;

    //
    // Prints evaluation result of the last operation
    //
    void print_result(command c) noexcept;

    //
    // Lists the entire source
    //
    void list_code(command c) noexcept;

    //
    // Prints a part of the ast
    //
    void print_ast(command c) noexcept;

    //
    // Prints all currently available variables with values
    //
    void print_vars(command c) noexcept;

  private: // Utility
    //
    // Returns a reference to the in stream
    //
    in_stream& in() noexcept;

    //
    // Returns a reference to the out stream
    //
    out_stream& out() noexcept;

    //
    // Returns a reference to the err stream
    //
    out_stream& err() noexcept;

    //
    // Attempts to open a file by a filename stored in the given token
    // If successful, sets the output stream
    // Otherwise, issues an error and leaves the current stream intact
    // The caller must maintain a value guard to restore the previous out stream
    //
    void try_redirect_output(const tnac::token& pathTok) noexcept;

    //
    // Cloases the temporary output file (if applicable)
    //
    void end_redirect() noexcept;

    //
    // Declares supported commands
    //
    void init_commands() noexcept;

    //
    // Stores a variable
    //
    void store_var(variable_ref var) noexcept;

    //
    // Parses input and executes commands
    //
    void parse(tnac::buf_t input, bool interactive) noexcept;

  private:
    parser m_parser;
    eval m_ev;

    src_manager m_srcMgr;

    tnac::commands::store m_commands;
    tnac::cmd m_cmd;

    in_stream*  m_in { &std::cin };
    out_stream* m_out{ &std::cout };
    out_stream* m_err{ &std::cerr };

    std::ofstream m_outFile{};

    var_collection m_vars;

    int m_numBase{ 10 };
    bool m_running{};
  };
}