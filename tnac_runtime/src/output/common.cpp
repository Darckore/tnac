#include "output/common.hpp"

namespace tnac_rt::out
{
  namespace
  {
    std::ostream& operator<<(std::ostream& out, eval::invalid_val_t) noexcept
    {
      out << "<undef>";
      return out;
    }

    std::ostream& operator<<(std::ostream& out, const tnac::complex_type& c) noexcept
    {
      out << "complex(" << c.real() << ", " << c.imag() << ')';
      return out;
    }
  }

  std::ostream& operator<<(std::ostream& out, const tnac::token& tok) noexcept
  {
    out << tok.m_value;
    return out;
  }

  // Special members

  value_printer::~value_printer() noexcept = default;

  value_printer::value_printer() noexcept = default;

  // Public members

  void value_printer::operator()(eval::value val, out_stream& os) noexcept
  {
    m_out = &os;
    print_value(val);
  }

  void value_printer::operator()(eval::value val) noexcept
  {
    this->operator()(val, out());
  }

  // Private members

  out_stream& value_printer::out() noexcept
  {
    return *m_out;
  }

  void value_printer::print_value(eval::value val) noexcept
  {
    eval::on_value(val, [this](auto val)
      {
        out() << val;
      });
  }
}