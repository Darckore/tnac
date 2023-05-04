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

    std::ostream& operator<<(std::ostream& out, const tnac::fraction_type& f) noexcept
    {
      if (f.sign() < 0)
        out << '-';

      if (f.is_infinity())
      {
        out << std::numeric_limits<tnac::float_type>::infinity();
        return out;
      }

      const auto den = f.denom();
      auto num = f.num();
      if (den == tnac::int_type{ 1 })
      {
        out << num;
        return out;
      }

      auto wholePart = num / den;

      if (wholePart)
      {
        out << wholePart;
        num = num % den;
        if (!num)
          return out;

        out << '(';
      }

      out << num << '/' << den;

      if (wholePart)
        out << ')';

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

  void value_printer::operator()(eval::value val, int base, out_stream& os) noexcept
  {
    m_out = &os;

    tnac::value_guard _{ m_base, base };
    print_value(val);
  }

  void value_printer::operator()(eval::value val, int base) noexcept
  {
    this->operator()(val, base, out());
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
        if constexpr (tnac::is_same_noquals_v<decltype(val), tnac::int_type>)
        {
          if (m_base == 10)
          {
            out() << val;
            return;
          }

          tnac::buf_t conv;
          static constexpr auto byteSizeInBin = 8u;
          static constexpr auto byteSizeInOct = 3u;
          static constexpr auto byteSizeInHex = 2u;
          static constexpr auto intSize = sizeof(tnac::int_type);

          switch (m_base)
          {
          case 2:
            conv.resize(byteSizeInBin * intSize);
            out() << "0b";
            break;
          case 8:
            conv.resize(byteSizeInOct * intSize);
            out() << '0';
            break;
          case 16:
            conv.resize(byteSizeInHex * intSize);
            out() << "0x";
            break;
          }

          auto basePtr = conv.data();
          std::to_chars(basePtr, basePtr + conv.size(), val, m_base);
          out() << conv;
        }
        else
        {
          out() << val;
        }
      });
  }
}