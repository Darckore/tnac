#include "parser/lex.hpp"

namespace tnac
{
  // Special members

  lex::lex(buf_t buf) noexcept
  {
    feed(std::move(buf));
  }


  // Public members

  void lex::feed(buf_t buf) noexcept
  {
    m_buf = std::move(buf);
    m_from = m_buf.begin();
    m_to   = m_from;
  }

  token lex::next() noexcept
  {
    if (!good())
      return { .m_pos{}, .m_kind{ token::Eol } };

    return {};
  }

  // Private members

  bool lex::good() const noexcept
  {
    return m_from != m_buf.end();
  }

}