#include "eval/console.hpp"

namespace tnac::eval
{
  // Special members

  console::~console() noexcept = default;

  console::console() noexcept = default;


  // Statics

  void console::write_int(rt::out_stream& stream, eval::int_type val, int base) noexcept
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
    static constexpr auto intSize = sizeof(eval::int_type);

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

    using uint = std::make_unsigned_t<eval::int_type>;
    auto outVal = std::bit_cast<uint>(val);
    auto basePtr = conv.data();
    std::to_chars(basePtr, basePtr + conv.size(), outVal, base);
    stream << conv;
  }

  void console::write_float(rt::out_stream& stream, eval::float_type val) noexcept
  {
    stream << val;
  }

  void console::write_bool(rt::out_stream& stream, eval::bool_type val, bool asStr) noexcept
  {
    if (!asStr)
    {
      stream << val;
      return;
    }

    stream << (val ? "true" : "false");
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
      [&](auto val) noexcept
      {
        utils::unused(val);
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