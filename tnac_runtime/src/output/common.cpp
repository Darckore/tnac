#include "output/common.hpp"
#include "sema/symbol.hpp"

namespace tnac::rt::out
{
  namespace
  {
    std::ostream& operator<<(std::ostream& out, eval::invalid_val_t) noexcept
    {
      out << "<undef>";
      return out;
    }

    std::ostream& operator<<(std::ostream& out, const eval::complex_type& c) noexcept
    {
      const auto r = c.real();
      const auto i = c.imag();

      out << '(';
      out << r;
      auto sign = (i > 0) ? '+' : '-';
      out << ' ' << sign << ' ' << utils::abs(i) << "i";
      out << ')';
      return out;
    }

    std::ostream& operator<<(std::ostream& out, const eval::fraction_type& f) noexcept
    {
      if (f.sign() < 0)
        out << '-';

      if (f.is_infinity())
      {
        out << std::numeric_limits<eval::float_type>::infinity();
        return out;
      }

      const auto den = f.denom();
      auto num = f.num();
      if (den == eval::int_type{ 1 })
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

    std::ostream& operator<<(std::ostream& out, const eval::function_type& f) noexcept
    {
      out << "function(" << f->name() << ')';

      return out;
    }
  }

  std::ostream& operator<<(std::ostream& out, const token& tok) noexcept
  {
    out << tok.value();
    return out;
  }

  // Special members

  value_printer::~value_printer() noexcept = default;

  value_printer::value_printer() noexcept = default;

  // Public members

  void value_printer::operator()(eval::value val, int base, out_stream& os) noexcept
  {
    m_out = &os;

    VALUE_GUARD(m_base, base);
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
        using vt = decltype(val);
        using eval::int_type;
        using eval::bool_type;
        using eval::array_type;
        if constexpr (utils::same_noquals<vt, int_type>)
        {
          if (m_base == 10)
          {
            out() << val;
            return;
          }

          buf_t conv;
          static constexpr auto byteSizeInBin = 8u;
          static constexpr auto byteSizeInOct = 3u;
          static constexpr auto byteSizeInHex = 2u;
          static constexpr auto intSize = sizeof(eval::int_type);

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

          using uint = std::make_unsigned_t<eval::int_type>;
          auto outVal = std::bit_cast<uint>(val);
          auto basePtr = conv.data();
          std::to_chars(basePtr, basePtr + conv.size(), outVal, m_base);
          out() << conv;
        }
        else if constexpr (utils::same_noquals<vt, bool_type>)
        {
          out() << (val ? "_true" : "_false");
        }
        else if constexpr (utils::same_noquals<vt, array_type>)
        {
          out() << "[ ";
          for (auto arrSz = val->size(); auto&& elem : *val)
          {
            --arrSz;
            print_value(*elem);
            if (arrSz)
              out() << ", ";
          }
          out() << " ]";
        }
        else
        {
          out() << val;
        }
      });
  }
}