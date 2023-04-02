//
// Driver
//

#pragma once
#include "ast/ast_nodes.hpp"
#include "parser/lex.hpp"
#include "parser/parser.hpp"
#include "evaluator/evaluator.hpp"
#include "evaluator/value_registry.hpp"
#include "sema/sema.hpp"

namespace tnac
{
  //
  // Driver for the entire system. Holds all the things needed to parse code,
  // executes commands, and manages the underlying buffers
  //
  class driver
  {
  public:
    CLASS_SPECIALS_NONE(driver);

    ~driver() noexcept;
  
    //
    // Runs interactively communicating via the given streams
    //
    driver(std::ostream& outStream, std::istream& inStream) noexcept;

  private: // Interactive mode
    void run() noexcept;

    void invite() noexcept;
    std::size_t read_line(std::size_t counter) noexcept;
    bool parse_line(std::size_t counter) noexcept;

  private:
    std::ostream& m_outStream;
    std::istream& m_inStream;

    ast::node* m_lastParsed{};

    ast::builder m_builder;
    sema m_sema;
    parser m_parser;
    eval::registry m_registry;

    std::unordered_map<std::size_t, buf_t> m_inputBuf;
  };
}