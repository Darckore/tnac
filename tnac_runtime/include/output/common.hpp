//
// Common definitions
//

#pragma once
#include "parser/ast/ast_visitor.hpp"
#include "cfg/cfg_walker.hpp"

namespace tnac
{
  namespace eval
  {
    class value;
    struct invalid_val_t;
  }
}

namespace tnac::rt::out
{
  //
  // Value printer
  //
  class value_printer final
  {
  public:
    CLASS_SPECIALS_NONE_CUSTOM(value_printer);

    ~value_printer() noexcept;

    value_printer() noexcept;

  public:
    void operator()(const eval::value& val, int base, out_stream& os) noexcept;

    void operator()(const eval::value& val, int base) noexcept;

  private:
    out_stream& out() noexcept;

    void print_value(const eval::value& val) noexcept;

  private:
    out_stream* m_out{ &std::cout };
    int m_base{ 10 };
  };
}