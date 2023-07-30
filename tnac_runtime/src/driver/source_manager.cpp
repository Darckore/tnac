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
    inputData.init_range();
    return inputData;
  }

  src_manager::stored_input* src_manager::from_file(tnac::string_t fname) noexcept
  {
    fsys::path fn{ fname };
    std::error_code errc;
    fn = fsys::absolute(fn, errc);
    if (errc)
    {
      err() << "Path '" << fn.string() << "' is invalid. " << errc.message() << '\n';
      return {};
    }

    tnac::buf_t buf;
    std::ifstream in{ fn.string() };
    if (!in)
    {
      err() << "Unable to open the input file '" << fn.string() << "'\n";
      return {};
    }

    in.seekg(0, std::ios::end);
    buf.reserve(in.tellg());
    in.seekg(0, std::ios::beg);

    using it = std::istreambuf_iterator<tnac::buf_t::value_type>;
    buf.assign(it{ in }, it{});
    return &input(std::move(buf));
  }

  void src_manager::on_error(const tnac::token& tok, tnac::string_t msg) noexcept
  {
    if (tok.is_eol())
    {
      err() << msg << '\n';
      return;
    }

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

  tnac::string_t src_manager::input_by(const tnac::token& tok) noexcept
  {
    auto&& last = m_input[m_prevIdx];
    if (last.in_buffer(tok))
      return last.m_buf;

    tnac::string_t ret;
    for (auto&& it : m_input)
    {
      auto&& inputData = it.second;
      if (inputData.in_buffer(tok))
      {
        ret = inputData.m_buf;
        break;
      }
    }

    return ret;
  }

  src_manager::src_loc src_manager::token_pos(const tnac::token& tok) noexcept
  {
    using sz_t = std::uintptr_t;
    auto toInt = [](auto ptr) noexcept
    {
      return reinterpret_cast<sz_t>(ptr);
    };

    static constexpr auto npos = tnac::string_t::npos;
    constexpr auto newLine = '\n';

    auto inputBuf = input_by(tok);
    auto getOffset = [l = toInt(tok.m_value.data()), &toInt](auto str) noexcept
    {
      const auto r = toInt(str.data());
      return static_cast<sz_t>(l - r);
    };

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