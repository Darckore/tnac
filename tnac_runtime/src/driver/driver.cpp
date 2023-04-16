#include "driver/driver.hpp"
#include "output/printer.hpp"

namespace tnac_rt
{
  // Special members

  driver::driver() noexcept :
    m_parser{ m_builder, m_sema }
  {
    m_parser.on_error([this](auto&& err) noexcept { on_parse_error(err); });
  }


  // Public members

  void driver::run_interactive() noexcept
  {
    tnac::buf_t input;
    m_running = true;

    while(m_running)
    {
      out() << ">> ";
      std::getline(in(), input);
      if (utils::ltrim(input).empty())
      {
        out() << "Enter an expression\n";
        continue;
      }

      parse(std::move(input));
      input = {};
    }
  }

  // Protected members

  driver& driver::set_istream(in_stream& stream) noexcept
  {
    m_in = &stream;
    return *this;
  }

  driver& driver::set_ostream(out_stream& stream) noexcept
  {
    m_out = &stream;
    return *this;
  }

  // Friends

  out_stream& operator<<(out_stream& stream, driver& drv) noexcept
  {
    drv.set_ostream(stream);
    return stream;
  }

  in_stream& operator>>(in_stream& stream, driver& drv) noexcept
  {
    drv.set_istream(stream);
    return stream;
  }

  // Private members

  in_stream& driver::in() noexcept
  {
    return *m_in;
  }
  out_stream& driver::out() noexcept
  {
    return *m_out;
  }
  out_stream& driver::err() noexcept
  {
    return *m_err;
  }

  tnac::string_t driver::get_current_input() noexcept
  {
    return m_input[m_inputIdx].m_buf;
  }

  driver::src_loc driver::token_pos(const tnac::token& tok) noexcept
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

  void driver::parse(tnac::buf_t input) noexcept
  {
    auto&& inputData = m_input[m_inputIdx] = { std::move(input) };
    auto ast = m_parser(inputData.m_buf);
    inputData.m_node = ast;
    
    eval ev{ m_registry };
    ev(ast);
    print_result();

    ++m_inputIdx;
  }

  void driver::print_result() noexcept
  {
    out() << '\n';
    out() << "Result: ";
    out::value_printer vp;
    vp(m_registry.evaluation_result(), out());
    out() << "\n\n";
  }

  void driver::on_parse_error(const tnac::ast::error_expr& error) noexcept
  {
    auto tokenPos = token_pos(error.at());
    
    err() << '\n' << tokenPos.m_line << '\n';
    for (auto off = std::size_t{ 1 }; off < tokenPos.m_offset; ++off)
      err() << ' ';

    err() << "^~~ " << error.message() << '\n';
  }
}