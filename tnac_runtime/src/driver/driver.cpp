#include "driver/driver.hpp"
#include "output/printer.hpp"

namespace tnac_rt
{
  // Special members

  driver::driver() noexcept :
    m_parser{ m_builder, m_sema }
  {}


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

  void driver::parse(tnac::buf_t input) noexcept
  {
    auto&& inputData = m_input[m_inputIdx] = { std::move(input) };
    auto ast = m_parser(inputData.m_buf);
    inputData.m_node = ast;
    
    eval ev{ m_registry };
    ev(ast);

    out() << '\n';
    out::ast_printer pr;
    pr(m_parser.root(), out());
    out() << '\n';

    ++m_inputIdx;
  }
}