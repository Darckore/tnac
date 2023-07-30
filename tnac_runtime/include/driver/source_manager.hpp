//
// Source manager
//

#pragma once
#include "ast/ast_nodes.hpp"
#include "ast/ast_visitor.hpp"

namespace tnac_rt
{
  //
  // The input data stored inside the source manager's collection
  //
  class input final
  {
  public:
    using buf_t  = tnac::buf_t;
    using str_t  = tnac::string_t;
    using path_t = fsys::path;

  public:
    CLASS_SPECIALS_NONE(input);

    input(buf_t buf, path_t inFile) noexcept :
      m_buffer{ std::move(buf) },
      m_file{ std::move(inFile) }
    {}

    input(buf_t buf) noexcept :
      input{ std::move(buf), path_t{ "*interactive*" }}
    {}

  public:
    str_t buffer() const noexcept
    {
      return m_buffer;
    }

  private:
    buf_t m_buffer;
    path_t m_file;
  };

  //
  // Manages the input buffer and reports errors
  //
  class src_manager final
  {
  public:
    using stored_input  = tnac_rt::input;
    using input_storage = std::unordered_map<std::uint32_t, input>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(src_manager);

    ~src_manager() noexcept;

    src_manager() noexcept;

  public:
    //
    // Stores the next input string
    //
    stored_input& input(tnac::buf_t in) noexcept;

    //
    // Reads input from a file and stores it
    //
    stored_input* from_file(tnac::string_t fname) noexcept;

    //
    // Prints an error message by token pos
    //
    void on_error(const tnac::token& tok, tnac::string_t msg) noexcept;

    //
    // Handler for parse errors
    //
    void on_parse_error(const tnac::ast::error_expr& err) noexcept;

  private:
    //
    // Returns a reference to the err stream
    //
    out_stream& err() noexcept;

  private:
    input_storage m_input;
    std::uint32_t m_prevIdx{};
    std::uint32_t m_inputIdx{};

    out_stream* m_err{ &std::cerr };
  };
}