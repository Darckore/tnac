//
// Driver
//

#pragma once
#include "parser/lex.hpp"
#include "parser/parser.hpp"
#include "evaluator/evaluator.hpp"
#include "evaluator/value_registry.hpp"
#include "sema/sema.hpp"
#include "driver/source_manager.hpp"
#include "driver/cmd_interpreter.hpp"

namespace tnac_rt
{
  //
  // System driver
  // Reads input and evaluates expressions
  //
  class driver
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
    using ast_builder = tnac::ast::builder;
    using sema        = tnac::sema;
    using parser      = tnac::parser;
    using val_reg     = tnac::eval::registry;
    using eval        = tnac::evaluator;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(driver);

    driver() noexcept;

  public:
    //
    // Runs in interactive mode until the #exit command is issued
    //
    void run_interactive() noexcept;

  protected:
    //
    // Sets the stream input will be read from
    //
    driver& set_istream(in_stream& stream) noexcept;

    //
    // Sets the stream for output
    //
    driver& set_ostream(out_stream& stream) noexcept;

  private: // Command handlers
    //
    // #exit
    //
    void on_exit() noexcept;

    //
    // Prints evaluation result of the last operation
    //
    void print_result() noexcept;

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
    // Declares supported commands
    //
    void init_commands() noexcept;

    //
    // Parses input and executes commands
    //
    void parse(tnac::buf_t input, bool interactive) noexcept;

  private:
    ast_builder m_builder;
    sema m_sema;
    val_reg m_registry;

    parser m_parser;
    src_manager m_srcMgr;

    commands::store m_commands;
    cmd m_cmd;

    in_stream*  m_in { &std::cin };
    out_stream* m_out{ &std::cout };
    out_stream* m_err{ &std::cerr };

    bool m_running{};
  };
}