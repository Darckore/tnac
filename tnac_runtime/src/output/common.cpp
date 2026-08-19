#include "output/common.hpp"
#include "eval/value/value.hpp"
#include "eval/console.hpp"

namespace tnac::rt::out
{
  // Special members

  value_printer::~value_printer() noexcept = default;

  value_printer::value_printer() noexcept = default;

  // Public members

  void value_printer::operator()(const eval::value& val, int base, out_stream& os) noexcept
  {
    m_out = &os;

    VALUE_GUARD(m_base, base);
    print_value(val);
  }

  void value_printer::operator()(const eval::value& val, int base) noexcept
  {
    this->operator()(val, base, out());
  }

  // Private members

  out_stream& value_printer::out() noexcept
  {
    return *m_out;
  }

  void value_printer::print_value(const eval::value& val) noexcept
  {
    using namespace eval;
    utils::visitor v
    {
      [&](int_type val) noexcept
      {
        console::write_int(out(), val, 10);
      },
      [&](float_type val) noexcept
      {
        console::write_float(out(), val);
      },
      [&](bool_type val) noexcept
      {
        console::write_bool(out(), val, true);
      },
      [&](fraction_type val) noexcept
      {
        console::write_fraction(out(), val, false);
      },
      [&](complex_type val) noexcept
      {
        console::write_complex(out(), val, false);
      },
      [&](array_type val) noexcept
      {
        console::write_array(out(), val);
      },
      [&](function_type val) noexcept
      {
        console::write_function(out(), val, true);
      },
      [&](auto) noexcept
      {
        console::write_undef(out());
      }
    };
    on_value(val, v);
  }
}