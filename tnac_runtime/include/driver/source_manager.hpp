//
// Source manager
//

#pragma once
#include "ast/ast_nodes.hpp"
#include "ast/ast_visitor.hpp"

namespace tnac_rt
{
  namespace detail
  {
    template <typename T>
    concept ast_printer = std::is_base_of_v<tnac::ast::const_top_down_visitor<T>, T>;
  }

  //
  // Manages the input buffer and reports errors
  //
  class src_manager final
  {
  public:
    //
    // Location of a token in input
    // Contains a source line and offset
    //
    struct src_loc
    {
      tnac::string_t m_line{};
      std::size_t    m_offset{};
    };

    struct stored_input
    {
      tnac::buf_t m_buf;
      tnac::ast::node* m_node{};

      std::uintptr_t m_begin{};
      std::uintptr_t m_end{};

      void init_range() noexcept
      {
        m_begin = reinterpret_cast<std::uintptr_t>(m_buf.data());
        m_end   = m_begin + static_cast<std::uintptr_t>(m_buf.size());
      }

      bool in_buffer(const tnac::token& tok) const noexcept
      {
        const auto tokPos = reinterpret_cast<std::uintptr_t>(tok.m_value.data());
        return utils::in_range(tokPos, m_begin, m_end);
      }
    };

    using input_storage = std::unordered_map<std::uint32_t, stored_input>;

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

    //
    // Retrieves the last parsed ast
    //
    const tnac::ast::node* last_parsed() noexcept;

  private:
    //
    // Returns a reference to the err stream
    //
    out_stream& err() noexcept;

    //
    // Gets input by token
    //
    tnac::string_t input_by(const tnac::token& tok) noexcept;

    //
    // Retrieves token position from input
    //
    src_loc token_pos(const tnac::token& tok) noexcept;

  private:
    input_storage m_input;
    std::uint32_t m_prevIdx{};
    std::uint32_t m_inputIdx{};

    out_stream* m_err{ &std::cerr };
  };
}