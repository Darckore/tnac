#pragma once
#include "parser/parser.hpp"
#include "sema/sema.hpp"
#include "eval/value/value.hpp"
#include "eval/types/traits.hpp"
#include "compiler/src_mgr/source_manager.hpp"
#include "parser/commands/cmd_interpreter.hpp"

namespace tnac_tests
{
  //
  // Stuff
  //

  namespace tree = tnac::ast;
  using tree::node_kind;
  using tnac::string_t;
  using tnac::buf_t;
  using tnac::tok_kind;
  using tnac::ast::node_kind;
  using cplx = tnac::eval::complex_type;
  using frac = tnac::eval::fraction_type;
  using arr  = tnac::eval::array_type;
  using func = tnac::eval::function_type;

  inline testing::Message& operator<<(testing::Message& msg, const frac& f) noexcept
  {
    if (f.sign() < 0) msg << '-';
    msg << f.num() << ',' << f.denom();
    return msg;
  }
  inline testing::Message& operator<<(testing::Message& msg, const func& f) noexcept
  {
    msg << "function(" << f->name() << ')';
    return msg;
  }

  class tnac_core final
  {
  public:
    CLASS_SPECIALS_NONE(tnac_core);

    ~tnac_core() noexcept = default;

    tnac_core(std::size_t stackSize) noexcept :
      m_parser{ m_builder, m_sema },
      m_cmdInterpreter{ m_cmdStore }
    {
      utils::unused(stackSize);
    }

  public:
    tnac::parser& get_parser() noexcept
    {
      return m_parser;
    }

  private:
    tnac::ast::builder m_builder;
    tnac::sema m_sema;
    tnac::parser m_parser;
    tnac::source_manager m_srcMgr;

    tnac::commands::store m_cmdStore;
    tnac::cmd m_cmdInterpreter;
  };

  inline tnac_core get_tnac(std::size_t stackSz = 0) noexcept
  {
    return tnac_core{ stackSz };
  }

#if 0
  //
  // Evaluator
  //

  template <typename T>
  concept testable = tnac::eval::detail::generic_type<T>;

  class value_checker final
  {
  public:
    using value_type = tnac::eval::value;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(value_checker);

    static constexpr auto infinity() noexcept
    {
      return std::numeric_limits<tnac::eval::float_type>::infinity();
    }
    static constexpr auto nan() noexcept
    {
      return std::numeric_limits<tnac::eval::float_type>::quiet_NaN();
    }

    static auto from_file(string_t fname, bool doEval = false) noexcept
    {
      return value_checker{ fname, doEval };
    }

    template <testable T>
    static void verify(tnac::eval::value val, T expected) noexcept
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
              using tnac::eval::eq;
              ASSERT_TRUE(eq(expected, val)) << "expected: " << expected << " got: " << val;
            }
          });
      }
    }

    static void verify(tnac::eval::value val, arr expected) noexcept
    {
      auto cv = val.try_get<tnac::eval::array_type>();
      if (!cv)
      {
        FAIL() << "Checked value is not an array";
        return;
      }

      auto&& checked = *cv;
      if (checked->size() != expected->size())
      {
        FAIL() << "Wrong size. Expected: " << checked->size() << " got: " << expected->size();
        return;
      }

      for (auto&& [c, e] : utils::make_iterators(*checked, *expected))
      {
        auto checkedVal  = *c;
        auto expectedVal = *e;
        tnac::eval::on_value(expectedVal, [checkedVal](auto expVal) noexcept
          {
            verify(checkedVal, std::move(expVal));
          });
      }
    }

    template <testable T>
    static void check(string_t input, T expected) noexcept
    {
      value_checker checker{};
      checker(input, std::move(expected));
    }

    static void check(string_t input) noexcept
    {
      check(input, tnac::eval::invalid_val_t{});
    }

  private:
    static void on_eval_error(const tnac::token& tok, string_t msg) noexcept
    {
      FAIL() << "Eval error " << msg << " at " << tok.value();
    }
    static void on_parse_error(const tnac::ast::error_expr& err) noexcept
    {
      FAIL() << "Parse error " << err.message() << " at " << err.pos().value();
    }

  private:
    value_checker() noexcept :
      m_tnac{ 128 }
    {
      //m_tnac.on_parse_error(on_parse_error);
      //m_tnac.on_semantic_error(on_eval_error);
    }

    value_checker(string_t fname, bool doEval = false) noexcept :
      value_checker{}
    {
      read_file(fname);
      utils::unused(doEval);
      //if (doEval)
      //  eval(m_buffer);
    }

  public:
    //value_type eval(string_t input) noexcept
    //{
    //  return core().evaluate(input);
    //}

    template <testable T>
    void operator()(string_t input, T expected) noexcept
    {
      auto res = eval(input);
      verify(res, std::move(expected));
    }

    template <testable T>
    void operator()(T expected) noexcept
    {
      operator()(m_buffer, std::move(expected));
    }

    void operator()(string_t input) noexcept
    {
      operator()(input, tnac::eval::invalid_val_t{});
    }

    void operator()() noexcept
    {
      operator()(m_buffer);
    }

  private:
    void read_file(string_t fname) noexcept
    {
      fsys::path fn{ fname };

      std::error_code errc;
      fn = fsys::absolute(fn, errc);
      ASSERT_FALSE(static_cast<bool>(errc)) << "Bad file name " << fname;
      std::ifstream in{ fn.string() };
      ASSERT_TRUE(static_cast<bool>(in));
      in.seekg(0, std::ios::end);
      m_buffer.reserve(in.tellg());
      in.seekg(0, std::ios::beg);
      using it = std::istreambuf_iterator<tnac::buf_t::value_type>;
      m_buffer.assign(it{ in }, it{});
    }

    tnac_core& core() noexcept
    {
      return m_tnac;
    }

  private:
    tnac_core m_tnac;
    buf_t m_buffer;
  };

  inline auto read_program(string_t fname, bool doEval = false) noexcept
  {
    return value_checker::from_file(fname, doEval);
  }

  template <testable T>
  inline void verify_program(string_t fname, T expected) noexcept
  {
    auto vc = read_program(fname);
    vc(std::move(expected));
  }

  inline void verify_program(string_t fname) noexcept
  {
    verify_program(fname, tnac::eval::invalid_val_t{});
  }


  //
  // Arrays
  //

  class array_builder final
  {
  public:
    using value_type = tnac::eval::array_type;
    using arr_t      = value_type::value_type;
    using arr_store  = std::forward_list<arr_t>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(array_builder);

    array_builder() noexcept = default;

  public:
    arr_t& add(std::size_t prealloc) noexcept
    {
      auto&& arr = m_arrays.emplace_front();
      arr.reserve(prealloc);
      return arr;
    }

    value_type to_array_type(arr_t& arr) noexcept
    {
      return { arr, ~std::size_t{} };
    }

  private:
    arr_store m_arrays;
  };
#endif
}