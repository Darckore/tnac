#include "eval/console.hpp"
#include "eval/value/types.hpp"
#include "eval/value/type_impl.hpp"
#include "cfg/ir/ir.hpp"
#include "parser/lex.hpp"

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

  void console::write_array(rt::out_stream& stream, array_type val) noexcept
  {
    stream << "[ ";
    const auto end = val->end();
    for (auto it = val->begin(); it != val->end(); ++it)
    {
      write(stream, *it);
      stream << ' ';
    }
    stream << ']';
  }

  void console::write_function(rt::out_stream& stream, function_type val, bool printClosure) noexcept
  {
    stream << val->name() << '(' << val->param_count() << ')';
    if (!printClosure || !val.is_closure())
      return;

    stream << '[';
    auto&& rec  = val->rec();
    auto&& data = val.closure_data();
    UTILS_ASSERT(rec.size() == data.size());
    const auto recSz = rec.size();
    for (auto idx = 0ul; idx < recSz; ++idx)
    {
      stream << rec.get_element(idx)->name() << '=';
      write(stream, data.read_at(idx));
      if (idx + 1 != recSz)
        stream << ", ";
    }
    stream << ']';
  }

  void console::write(rt::out_stream& stream, const value& val) noexcept
  {
    utils::visitor v
    {
      [&](int_type val) noexcept
      {
        write_int(stream, val, 10);
      },
      [&](float_type val) noexcept
      {
        write_float(stream, val);
      },
      [&](bool_type val) noexcept
      {
        write_bool(stream, val, false);
      },
      [&](fraction_type val) noexcept
      {
        write_fraction(stream, val, false);
      },
      [&](complex_type val) noexcept
      {
        write_complex(stream, val, false);
      },
      [&](array_type val) noexcept
      {
        write_array(stream, val);
      },
      [&](function_type val) noexcept
      {
        write_function(stream, val, false);
      },
      [&](auto) noexcept
      {
        write_undef(stream);
      }
    };
    on_value(val, v);
  }


  // Public members

  void console::write(const value& val) noexcept
  {
    write(out(), val);
  }

  value console::read() noexcept
  {
    value res{};

    buf_t input;
    std::getline(in(), input);
    lex l;
    l(input);

    auto piece = l.next();
    auto op = val_ops::UnaryPlus;
    if (piece.is(token::Minus))
    {
      op = val_ops::UnaryNegation;
      piece = l.next();
    }
    else if (piece.is(token::Plus))
      piece = l.next();

    auto tokStr = piece.value();
    switch (piece.what())
    {
    case token::IntBin: res = value::parse_int(tokStr, 2).unary(op);  break;
    case token::IntOct: res = value::parse_int(tokStr, 8).unary(op);  break;
    case token::IntHex: res = value::parse_int(tokStr, 16).unary(op); break;
    case token::IntDec: res = value::parse_int(tokStr, 10).unary(op); break;
    case token::Float:  res = value::parse_float(tokStr).unary(op);   break;
    
    default: break;
    }

    return res;
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