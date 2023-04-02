#include "tnac/driver.hpp"

namespace tnac
{
  // Special members

  driver::~driver() noexcept = default;

  driver::driver(std::ostream& outStream, std::istream& inStream) noexcept :
    m_outStream{ outStream },
    m_inStream{ inStream },
    m_parser{ m_builder, m_sema }
  {
    run();
  }


  // Private members

  void driver::run() noexcept
  {
    for (auto inputCounter = std::size_t{}; ; )
    {
      invite();
      auto nextStep = read_line(inputCounter);
      if (nextStep == inputCounter)
        continue;

      if (!parse_line(inputCounter))
        return;

      inputCounter = nextStep;
    }
  }

  void driver::invite() noexcept
  {
    m_outStream << ">>> ";
  }
  std::size_t driver::read_line(std::size_t counter) noexcept
  {
    buf_t input;
    std::getline(m_inStream, input);
    if (utils::ltrim(input).empty())
    {
      m_outStream << "Enter an expression\n";
      return counter;
    }

    m_inputBuf[counter] = std::move(input);

    return counter + 1;
  }
  bool driver::parse_line(std::size_t counter) noexcept
  {
    m_lastParsed = m_parser(m_inputBuf[counter]);
    evaluator{ m_registry }(m_lastParsed);

    return true;
  }

}