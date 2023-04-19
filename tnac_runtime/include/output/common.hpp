//
// Common definitions
//

#pragma once
#include "ast/ast_visitor.hpp"
#include "evaluator/value.hpp"

namespace tnac_rt::out
{
  namespace ast = tnac::ast;
  namespace eval = tnac::eval;

  //
  // Printer base class
  //
  template <typename D>
  using printer_base = ast::const_top_down_visitor<D>;

  //
  // Token output
  //
  std::ostream& operator<<(std::ostream& out, const tnac::token& tok) noexcept;

  //
  // Value printer
  //
  class value_printer
  {
  public:
    CLASS_SPECIALS_NONE_CUSTOM(value_printer);

    ~value_printer() noexcept;

    value_printer() noexcept;

  public:
    void operator()(eval::value val, out_stream& os) noexcept;

    void operator()(eval::value val) noexcept;

  private:
    out_stream& out() noexcept;

    void print_value(eval::value val) noexcept;

  private:
    out_stream* m_out{ &std::cout };
  };
}