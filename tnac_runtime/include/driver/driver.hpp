//
// Driver
//

#pragma once
#include "packages/core.hpp"
#include "driver/input_mgr.hpp"

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
    using variable_ptr   = const tnac::semantics::variable*;
    using variable_ref   = const tnac::semantics::variable&;
    using var_collection = std::vector<variable_ptr>;
    using command        = tnac::ast::command;
    using size_type      = command::size_type;
    using input_t        = input_mgr::stored_input;

    using core = tnac::packages::tnac_core;

  private:
    struct io
    {
      in_stream*  in{ &std::cin };
      out_stream* out{ &std::cout };
      out_stream* err{ &std::cerr };

      std::ofstream outFile{};

      bool is_in_cout() const noexcept
      {
        return out == &std::cout;
      }
    };
    
    struct state
    {
      CLASS_SPECIALS_ALL(state);

      int  numBase{ 10 };
      bool running{};
      bool interactive{};
      bool compile{};
      bool optimise{};
      tnac::ast::node* lastParsed{};

      explicit operator bool() const noexcept
      {
        return running;
      }

      void start() noexcept
      {
        running = true;
      }
      void stop() noexcept
      {
        running = false;
      }
    };

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
    // Parses the command line and launches the driver in the selected run mode
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

    //
    // Prints variable scope
    //
    void print_var_scope(variable_ref var, bool styles) noexcept;

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
    // Closes the temporary output file (if applicable)
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
    // Processes the parsed code
    //
    void process(tnac::ast::node* root) noexcept;

    //
    // Parses input and executes commands
    //
    void parse(input_t& inputData) noexcept;

  private:
    core m_tnac;
    io m_io{};
    state m_state{};

    input_mgr m_inpMgr;
    var_collection m_vars;
  };
}