#pragma once
#include "packages/core.hpp"

namespace tnac_tests
{
  namespace detail
  {
    namespace tree = tnac::ast;
    using tree::node_kind;
    using tnac::string_t;

    struct parse_helper
    {
      parse_helper() :
        parser{ builder, sema }
      {}

      auto operator()(string_t input) noexcept
      {
        return parser(input);
      }

      tnac::ast::builder builder;
      tnac::sema sema;
      tnac::parser parser;
    };

    using cplx = tnac::eval::complex_type;
    using frac = tnac::eval::fraction_type;

    template <tnac::eval::detail::expr_result T>
    inline bool eq(const T& l, const T& r) noexcept
    {
      return l == r;
    }

    template <>
    inline bool eq(const tnac::eval::float_type& l, const tnac::eval::float_type& r) noexcept
    {
      if (std::isinf(l) && std::isinf(r))
        return true;

      if (std::isnan(l) && std::isnan(r))
        return true;

      return utils::eq(l, r);
    }

    template <>
    inline bool eq(const cplx& l, const cplx& r) noexcept
    {
      return eq(l.real(), r.real()) && eq(l.imag(), r.imag());
    }

    inline testing::Message& operator<<(testing::Message& msg, const frac& f) noexcept
    {
      if (f.sign() < 0) msg << '-';
      msg << f.num() << ',' << f.denom();
      return msg;
    }

    inline auto parse_input(string_t input, tnac::eval::registry& reg, tnac::eval::call_stack& cs) noexcept
    {
      parse_helper p;
      p.parser(input);
      tnac::evaluator ev{ reg, cs };
      ev(p.parser.root());

      return reg.evaluation_result();
    }

    inline constexpr auto invalid_value() noexcept
    {
      return tnac::eval::invalid_val_t{};
    }

    template <typename T>
    concept testable_val = tnac::eval::detail::expr_result<T> ||
                           tnac::is_same_noquals_v<T, tnac::eval::invalid_val_t>;

    template <testable_val T>
    inline void verify(tnac::eval::value val, T expected) noexcept
    {
      if constexpr (tnac::is_same_noquals_v<T, tnac::eval::invalid_val_t>)
      {
        ASSERT_TRUE(!val);
      }
      else
      {
        tnac::eval::on_value(val, [expected](auto val) noexcept
          {
            using expected_t = decltype(val);
            if constexpr (tnac::is_same_noquals_v<expected_t, tnac::eval::invalid_val_t>)
            {
              ASSERT_TRUE(false) << "Undefined value detected";
            }
            else if constexpr (!tnac::is_same_noquals_v<expected_t, T>)
            {
              ASSERT_TRUE(false) << "Wrong value type";
            }
            else
            {
              ASSERT_TRUE(eq(expected, val)) << "expected: " << expected << " got: " << val;
            }
          });
      }
    }

    template <tnac::eval::detail::expr_result T>
    inline void check_eval(string_t input, T expected) noexcept
    {
      tnac::eval::registry reg;
      tnac::eval::call_stack cs{ 10 };
      verify(parse_input(input, reg, cs), expected);
    }

    inline void check_invalid(string_t input) noexcept
    {
      tnac::eval::registry reg;
      tnac::eval::call_stack cs{ 0 };
      verify(parse_input(input, reg, cs), invalid_value());
    }
  }
}