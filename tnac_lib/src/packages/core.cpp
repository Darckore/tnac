#include "packages/core.hpp"

namespace tnac::packages
{
  // Special members

  tnac_core::~tnac_core() noexcept = default;

  tnac_core::tnac_core(size_type stackSize) noexcept :
    m_ev{ stackSize }
  {}


  // Public members

  parser& tnac_core::get_parser() noexcept
  {
    return m_parser;
  }
  evaluator& tnac_core::get_eval() noexcept
  {
    return m_ev;
  }
  cmd& tnac_core::get_commands() noexcept
  {
    return m_cmd;
  }

  tnac_core::result_type tnac_core::evaluate(string_t input) noexcept
  {
    auto parsedAst = m_parser(input);
    m_ev(parsedAst);
    return m_ev.last_result();
  }
}