#include "parser/parser.hpp"

namespace tnac
{
  // Public members

  parser::pointer parser::parse(string_t str) noexcept
  {
    m_lex.feed(str);
    return nullptr;
  }

  parser::const_root_ptr parser::root() const noexcept
  {
    return m_root;
  }
  parser::root_ptr parser::root() noexcept
  {
    return utils::mutate(std::as_const(*this).root());
  }
}