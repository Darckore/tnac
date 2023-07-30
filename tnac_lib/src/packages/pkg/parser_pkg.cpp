#include "packages/pkg/parser_pkg.hpp"

namespace tnac::packages
{
  // Special members

  parser::~parser() noexcept = default;

  parser::parser() noexcept :
    m_parser{ m_builder, m_sema }
  {}


  // Public members

  parser::ast_ptr parser::operator()(string_t input) noexcept
  {
    return m_parser(input);
  }

  parser::ast_ptr parser::operator()(string_t input, loc srcLoc) noexcept
  {
    return m_parser(input, srcLoc);
  }

  parser::const_ast_root parser::root() const noexcept
  {
    return m_parser.root();
  }
  parser::ast_root parser::root() noexcept
  {
    return FROM_CONST(root);
  }

  source_manager& parser::get_src_mgr() noexcept
  {
    return m_srcMgr;
  }
}