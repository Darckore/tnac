#include "driver/source_manager.hpp"

namespace tnac_rt
{
  // Special members

  src_manager::~src_manager() noexcept = default;

  src_manager::src_manager() noexcept = default;

  // Public members

  src_manager::stored_input& src_manager::input(tnac::buf_t in) noexcept
  {
    m_prevIdx = m_inputIdx;
    auto&& inputData = m_input[m_inputIdx++] = { std::move(in) };
    return inputData;
  }

  void src_manager::on_error(const tnac::token& tok, tnac::string_t msg) noexcept
  {
    auto tokenPos = token_pos(tok);

    err() << '\n' << tokenPos.m_line << '\n';
    for (auto off = std::size_t{ 1 }; off < tokenPos.m_offset; ++off)
      err() << ' ';

    err() << "^~~ " << msg << '\n';
  }

  void src_manager::on_parse_error(const tnac::ast::error_expr& error) noexcept
  {
    on_error(error.at(), error.message());
  }

  const tnac::ast::node* src_manager::last_parsed() noexcept
  {
    auto idx = m_prevIdx;
    while (idx)
    {
      if (auto node = m_input[idx--].m_node; node && node->parent())
        return node;
    }

    return m_input[idx].m_node;
  }

  // Private members

  out_stream& src_manager::err() noexcept
  {
    return *m_err;
  }

  tnac::string_t src_manager::get_current_input() noexcept
  {
    return m_input[m_prevIdx].m_buf;
  }

  src_manager::src_loc src_manager::token_pos(const tnac::token& tok) noexcept
  {
    using sz_t = tnac::string_t::size_type;
    static constexpr auto npos = tnac::string_t::npos;
    constexpr auto newLine = '\n';

    auto inputBuf = get_current_input();
    auto getOffset = [l = tok.m_value.data()](auto r) noexcept
    {
      return static_cast<sz_t>(l - r.data());
    };

    // Token is guaranteed to be from this input buffer, so we are sure the offset is
    // positive and is within the buffer
    const auto offset = getOffset(inputBuf);
    auto lineBeg = inputBuf.rfind(newLine, offset);
    if (lineBeg == npos)
    {
      lineBeg = sz_t{};
    }

    const auto charCount = inputBuf.find(newLine, offset) - lineBeg;
    auto line = utils::trim(inputBuf.substr(lineBeg, charCount));
    const auto errOff = (!tok.is_eol()) ?
      1 + getOffset(line) :
      1 + line.length();

    return { line, errOff };
  }

}