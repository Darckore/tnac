//
// Source manager
//

#pragma once
#include "ast/ast_nodes.hpp"

namespace tnac_rt
{
  //
  // The input data stored inside the input manager's collection
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
      input{ std::move(buf), path_t{ "*repl*" } }
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
  class input_mgr final
  {
  public:
    using stored_input  = tnac_rt::input;
    using input_storage = std::unordered_map<std::uint32_t, input>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(input_mgr);

    ~input_mgr() noexcept;

    input_mgr() noexcept;

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

    //
    // Stores the input and returns a reference to it
    //
    template <typename ...Args>
    stored_input& store(Args&& ...args) noexcept
    {
      auto newItem = m_input.try_emplace(m_inputIdx++, std::forward<Args>(args)...);
      return newItem.first->second;
    }

  private:
    input_storage m_input;
    std::uint32_t m_inputIdx{};

    out_stream* m_err{ &std::cerr };
  };
}