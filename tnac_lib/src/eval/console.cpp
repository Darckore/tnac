#include "eval/console.hpp"

namespace tnac::eval
{
  // Special members

  console::~console() noexcept = default;

  console::console() noexcept = default;


  // Statics

  void console::write_undef(rt::out_stream& stream) noexcept
  {
    stream << "<undef>"sv;
  }

  void console::write_int(rt::out_stream& stream, int_type val, int base) noexcept
  {
    if (base == 10)
    {
      stream << val;
      return;
    }

    buf_t conv;
    static constexpr auto byteSizeInBin = 8u;
    static constexpr auto byteSizeInOct = 3u;
    static constexpr auto byteSizeInHex = 2u;
    static constexpr auto intSize = sizeof(int_type);

    switch (base)
    {
    case 2:
      conv.resize(byteSizeInBin * intSize);
      stream << "0b";
      break;
    case 8:
      conv.resize(byteSizeInOct * intSize);
      stream << '0';
      break;
    case 16:
      conv.resize(byteSizeInHex * intSize);
      stream << "0x";
      break;
    }

    using uint = std::make_unsigned_t<int_type>;
    auto outVal = std::bit_cast<uint>(val);
    auto basePtr = conv.data();
    std::to_chars(basePtr, basePtr + conv.size(), outVal, base);
    stream << conv;
  }

  void console::write_float(rt::out_stream& stream,float_type val) noexcept
  {
    stream << val;
  }

  void console::write_bool(rt::out_stream& stream, bool_type val, bool asStr) noexcept
  {
    if (!asStr)
    {
      stream << val;
      return;
    }

    stream << (val ? "true" : "false");
  }

  void console::write_fraction(rt::out_stream& stream, fraction_type val, bool extractWhole) noexcept
  {
    if (val.sign() < 0)
      stream << '-';

    const auto den = val.denom();
    auto num = val.num();
    if (!den)
    {
      if (extractWhole)
      {
        stream << std::numeric_limits<tnac::eval::float_type>::infinity();
        return;
      }
      extractWhole = false;
    }

    if (extractWhole && den == int_type{ 1 })
    {
      stream << num;
      return;
    }

    auto wholePart = extractWhole ? (num / den) : int_type{};
    if (extractWhole && wholePart)
    {
      stream << wholePart;
      num = num % den;
      if (!num)
        return;

      stream << '(';
    }

    stream << num << '/' << den;

    if (extractWhole && wholePart)
      stream << ')';
  }

  void console::write_complex(rt::out_stream& stream, complex_type val, bool dropZero) noexcept
  {
    const auto r = val.real();
    const auto i = val.imag();
    const auto writeReal = !dropZero || !utils::eq(r, float_type{});
    const auto writeImag = !dropZero || !utils::eq(i, float_type{});

    if (!writeReal && !writeImag)
    {
      stream << float_type{};
      return;
    }

    if(writeReal)
      stream << r;

    if (writeImag)
    {
      if (writeReal)
      {
        auto sign = (i >= 0) ? '+' : '-';
        stream << ' ' << sign << ' ';
      }
      stream << utils::abs(i) << "i";
    }
  }


  // Public members

  void console::write(const value& val) noexcept
  {
    utils::visitor v
    {
      [&](int_type val) noexcept
      {
        write_int(out(), val, 10);
      },
      [&](float_type val) noexcept
      {
        write_float(out(), val);
      },
      [&](bool_type val) noexcept
      {
        write_bool(out(), val, false);
      },
      [&](fraction_type val) noexcept
      {
        write_fraction(out(), val, false);
      },
      [&](complex_type val) noexcept
      {
        write_complex(out(), val, false);
      },
      [&](array_type val) noexcept
      {
        utils::unused(val);
      },
      [&](function_type val) noexcept
      {
        utils::unused(val);
      },
      [&](auto) noexcept
      {
        write_undef(out());
      }
    };
    on_value(val, v);
  }


  // Private members

  rt::out_stream& console::out() noexcept
  {
    return *m_out;
  }

  rt::in_stream& console::in() noexcept
  {
    return *m_in;
  }
}