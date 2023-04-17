//
// Driver
//

#pragma once
#include "ast/ast_nodes.hpp"
#include "ast/ast_visitor.hpp"
#include "parser/lex.hpp"
#include "parser/parser.hpp"
#include "evaluator/evaluator.hpp"
#include "evaluator/value_registry.hpp"
#include "sema/sema.hpp"

namespace tnac_rt
{
  namespace detail
  {
    template <typename T>
    concept ast_printer = std::is_base_of_v<tnac::ast::const_top_down_visitor<T>, T>;
  }

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

  private:
    struct stored_input
    {
      tnac::buf_t m_buf;
      tnac::ast::node* m_node{};
    };

    using input_storage = std::unordered_map<std::uint32_t, stored_input>;

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

  private:
    //
    // Location of a token in input
    // Contains a source line and offset
    //
    struct src_loc
    {
      tnac::string_t m_line{};
      std::size_t    m_offset{};
    };

  private:
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
    // Retrieves the currently parsed input string
    //
    tnac::string_t get_current_input() noexcept;

    //
    // Retrieves token position from input
    //
    src_loc token_pos(const tnac::token& tok) noexcept;

    //
    // Parses input and executes commands
    //
    void parse(tnac::buf_t input) noexcept;

    //
    // Prints evaluation result of the last operation
    //
    void print_result() noexcept;

    //
    // Prints an error message by token pos
    //
    void on_error(const tnac::token& tok, tnac::string_t msg) noexcept;

    //
    // Handler for parse errors
    //
    void on_parse_error(const tnac::ast::error_expr& err) noexcept;

  private:
    ast_builder m_builder;
    sema m_sema;
    val_reg m_registry;

    parser m_parser;

    input_storage m_input;

    in_stream*  m_in { &std::cin };
    out_stream* m_out{ &std::cout };
    out_stream* m_err{ &std::cerr };

    std::uint32_t m_inputIdx{};
    bool m_running{};
  };
}