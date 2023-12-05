#include "output/common.hpp"
#include "output/formatting.hpp"
#include "eval/value/value.hpp"
#include "sema/sym/symbols.hpp"

namespace tnac::rt::out
{
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