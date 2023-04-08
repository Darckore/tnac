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
    auto&& in  = *m_in;
    auto&& out = *m_out;
    m_running = true;

    while(m_running)
    {
      out << ">> ";
      std::getline(in, input);
      if (utils::ltrim(input).empty())
      {
        out << "Enter an expression\n";
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

  std::ostream& operator<<(std::ostream& stream, driver& drv) noexcept
  {
    drv.set_ostream(stream);
    return stream;
  }

  std::istream& operator>>(std::istream& stream, driver& drv) noexcept
  {
    drv.set_istream(stream);
    return stream;
  }


  // Private members

  void driver::parse(tnac::buf_t input) noexcept
  {
    auto&& inputData = m_input[m_inputIdx] = { std::move(input) };
    auto ast = m_parser(inputData.m_buf);
    inputData.m_node = ast;
    

    ++m_inputIdx;
  }
}