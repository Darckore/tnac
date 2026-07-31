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
    // Prints an invalid value
    //
    static void write_undef(rt::out_stream& stream) noexcept;

    //
    // Prints an int value with the specified base
    //
    static void write_int(rt::out_stream& stream, int_type val, int base) noexcept;

    //
    // Prints a float value
    //
    static void write_float(rt::out_stream& stream, float_type val) noexcept;

    //
    // Prints a bool value
    // The asStr parameter specifies whether to print the value as a number
    // or a true/false string representation
    //
    static void write_bool(rt::out_stream& stream, bool_type val, bool asStr) noexcept;

    //
    // Prints a fraction value
    // The extractWhole parameter specifies whether to print the value as is
    // or to extract the whole part if the nominator is greater than the denominator
    //
    static void write_fraction(rt::out_stream& stream, fraction_type val, bool extractWhole) noexcept;

    //
    // Prints a complex value
    // The dropZero param specifies whether or not to print zero parts of the value
    //
    static void write_complex(rt::out_stream& stream, complex_type val, bool dropZero) noexcept;

    //
    // Prints an array
    //
    static void write_array(rt::out_stream& stream, array_type val) noexcept;

    //
    // Prints a function
    //
    static void write_function(rt::out_stream& stream, function_type val) noexcept;

    //
    // Dispatches the printing to the given stream
    //
    static void write(rt::out_stream& stream, const value& val) noexcept;

  public:
    //
    // Writes the given value to the current output stream
    //
    void write(const value& val) noexcept;

    //
    // Reads a value from the current input stream
    // Returns undef on error
    //
    value read() noexcept;

  private:
    rt::out_stream& out() noexcept;

    rt::in_stream& in() noexcept;

  private:
    rt::out_stream* m_out{ &std::cout };
    rt::in_stream* m_in{ &std::cin };
  };
}