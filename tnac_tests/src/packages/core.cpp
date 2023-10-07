#include "packages/core.hpp"

namespace tnac::packages
{
  // Special members

  tnac_core::~tnac_core() noexcept = default;

  tnac_core::tnac_core(size_type stackSize) noexcept :
    m_ev{ stackSize },
    m_compiler{ stackSize }
  {}


  // Public members

  parser& tnac_core::get_parser() noexcept
  {
    return m_parser;
  }
  compiler& tnac_core::get_compiler() noexcept
  {
    return m_compiler;
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
    m_parser(input);
    m_ev(m_parser.root());
    return m_ev.last_result();
  }
}