//
// Console used for IO
//

#pragma once
#include "eval/value/value.hpp"

namespace tnac::eval
{
  //
  // Performs IO with the specified stream
  //
  class console
  {
  public:
    CLASS_SPECIALS_NONE_CUSTOM(console);

    ~console() noexcept;

    console() noexcept;

  public:
    //
    // Prints an int value with the specified base
    //
    static void write_int(rt::out_stream& stream, eval::int_type val, int base) noexcept;

    //
    // Prints a float value
    //
    static void write_float(rt::out_stream& stream, eval::float_type val) noexcept;

    //
    // Prints a bool value
    // The asStr parameter specifies whether to print the value as a number
    // or a true/false string representation
    //
    static void write_bool(rt::out_stream& stream, eval::bool_type val, bool asStr) noexcept;

  public:
    //
    // Writes the given value to the current output stream
    //
    void write(const value& val) noexcept;

  private:
    rt::out_stream& out() noexcept;

    rt::in_stream& in() noexcept;

  private:
    rt::out_stream* m_out{ &std::cout };
    rt::in_stream* m_in{ &std::cin };
  };
}