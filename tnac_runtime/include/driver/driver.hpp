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
    friend std::ostream& operator<<(std::ostream& stream, driver& drv) noexcept;

    //
    // Sets the input stream for the driver
    //
    friend std::istream& operator>>(std::istream& stream, driver& drv) noexcept;

  public:
    using ast_builder = tnac::ast::builder;
    using sema        = tnac::sema;
    using parser      = tnac::parser;
    using val_reg     = tnac::eval::registry;
    using eval        = tnac::evaluator;

    using in_stream  = std::istream;
    using out_stream = std::ostream;

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
    // Parses input and executes commands
    //
    void parse(tnac::buf_t input) noexcept;

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