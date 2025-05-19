#pragma once
#include "core/tnac.hpp"
#include "eval/value/value.hpp"
#include "eval/value/traits.hpp"
#include "eval/value/type_impl.hpp"
#include "eval/value/value_store.hpp"

namespace tnac::tests
{
  //
  // Stuff
  //

  using cplx  = eval::complex_type;
  using frac  = eval::fraction_type;
  using arr   = eval::array_type;
  using func  = eval::function_type;
  using dummy = eval::invalid_val_t;

  using value = eval::value;

  inline ::testing::Message& operator<<(::testing::Message& msg, const frac& f) noexcept
  {
    if (f.sign() < 0) msg << '-';
    msg << f.num() << ',' << f.denom();
    return msg;
  }
  inline ::testing::Message& operator<<(::testing::Message& msg, const func& f) noexcept
  {
    msg << "function(" << f->name() << ')';
    return msg;
  }

  inline core get_tnac(feedback& fb) noexcept
  {
    return core{ fb };
  }

  template <typename T>
  concept testable =
    utils::any_same_as<T, cplx, frac, arr, func, dummy, bool> ||
    utils::integer<T> ||
    utils::real<T>;

  namespace detail
  {
    template <typename T>
    inline auto cast(T value) noexcept
    {
      return value;
    }

    inline auto cast(utils::integer auto value) noexcept
    {
      return static_cast<eval::int_type>(value);
    }

    inline auto cast(utils::real auto value) noexcept
    {
      return static_cast<eval::float_type>(value);
    }

    inline auto to_value(testable auto val) noexcept
    {
      return value{ cast(val) };
    }

    inline bool eq(dummy, dummy) noexcept
    {
      return true;
    }
  }

  class value_checker final
  {
  public:
    using enum eval::val_ops;
    using stored_op = std::optional<eval::val_ops>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(value_checker);

    value_checker() noexcept = default;

    explicit value_checker(eval::val_ops op) noexcept :
      m_op{ op }
    {
    }

    explicit value_checker(value val) noexcept :
      m_value{ val }
    {
    }

    template <testable T>
    explicit value_checker(T val) noexcept :
      value_checker{ to_value(val) }
    {
    }

  public:
    template <testable T>
    value_checker& verify(T expected) noexcept
    {
      const auto target = detail::cast(expected);
      using target_t = std::remove_cvref_t<decltype(target)>;
      constexpr auto targetId = utils::type_to_id_v<target_t>;
      const auto curId = m_value.id();
      EXPECT_TRUE(curId == targetId)
        << "Wrong type. Expected "
        << value::id_str(targetId)
        << ", actual " << value::id_str(curId);

      if (curId == targetId)
      {
        eval::on_value(m_value, utils::visitor{
          [](auto) noexcept {},
          [](arr) noexcept {},
          [](dummy) noexcept {},
          [&](target_t val) noexcept
          {
            using eval::eq;
            EXPECT_TRUE(eq(target, val)) << "expected: " << expected << " got: " << val;
          } });
      }

      return *this;
    }

    value_checker& verify() noexcept
    {
      EXPECT_TRUE(!m_value) << "expected and invalid value, got " << m_value.id_str();
      return *this;
    }

    value_checker& verify(arr expected) noexcept
    {
      utils::unused(expected);
      return *this;
    }

    value_checker& with_op(eval::val_ops op) noexcept
    {
      m_op = op;
      return *this;
    }

    value_checker& with(testable auto val) noexcept
    {
      m_value = detail::to_value(val);
      return *this;
    }

  public:
    value_checker& act(eval::val_ops op, testable auto val) noexcept
    {
      m_value = m_value.binary(op, detail::to_value(val));
      return *this;
    }

    value_checker& act(testable auto val) noexcept
    {
      if (m_op)
        return act(*m_op, val);
      return *this;
    }

    value_checker& act(eval::val_ops op) noexcept
    {
      m_value = m_value.unary(op);
      return *this;
    }

    value_checker& act() noexcept
    {
      if (m_op)
        return act(*m_op);
      return *this;
    }

  private:
    value m_value{};
    stored_op m_op{};
  };


  constexpr auto infinity() noexcept
  {
    return std::numeric_limits<eval::float_type>::infinity();
  }

  constexpr auto nan() noexcept
  {
    return std::numeric_limits<eval::float_type>::quiet_NaN();
  }


  class array_builder final
  {
  public:
    using arr_store = eval::store;
    using arr       = eval::array_data;
    using size_type = arr::size_type;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(array_builder);

    array_builder() noexcept = default;

  public:
    array_builder& with_new(size_type sz) noexcept
    {
      m_cur = &m_arrays.allocate_array(sz);
      return *this;
    }

    array_builder& add(testable auto val) noexcept
    {
      UTILS_ASSERT(m_cur);
      m_cur->add(detail::to_value(val));
      return *this;
    }

    eval::array_type get() noexcept
    {
      UTILS_ASSERT(m_cur);
      auto&& wrapper = m_arrays.wrap(*m_cur);
      return eval::array_type{ wrapper };
    }

  private:
    arr_store m_arrays;
    arr* m_cur{};
  };


  class source_tester final
  {
  public:
    CLASS_SPECIALS_NONE(source_tester);

    explicit source_tester(string_t path) noexcept :
      m_core{ m_fb }
    {
      const auto loadRes = m_core.process_file(path);
      EXPECT_TRUE(loadRes);

      m_core.compile();
      auto&& cfg = m_core.get_cfg();
      auto it = cfg.begin();
      EXPECT_NE(it, cfg.end());
    }

    source_tester& test(string_t input, testable auto expected) noexcept
    {
      utils::unused(input, expected);
      return *this;
    }

  private:
    feedback m_fb;
    core m_core;
  };

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
#endif
}