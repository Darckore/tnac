#include "parser/lex.hpp"

namespace tnac
{
  // Special members

  lex::lex(string_t buf) noexcept
  {
    feed(buf);
  }


  // Public members

  void lex::feed(string_t buf) noexcept
  {
    m_buf = buf;
    m_from = m_buf.begin();
    m_to   = m_from;
  }

  token lex::next() noexcept
  {
    if (!good())
      return { .m_value{}, .m_kind{ token::Eol } };

    return {};
  }

  // Private members

  bool lex::good() const noexcept
  {
    return m_from != m_buf.end();
  }

}