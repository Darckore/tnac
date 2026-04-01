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
            EXPECT_TRUE(eq(target, val)) << "expected " << expected << " got " << val;
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
      auto base = m_value.try_get<eval::type_id::Array>();
      if (!base)
      {
        EXPECT_TRUE(false) << "expected an array, got " << m_value.id_str();
        return *this;
      }

      auto&& stored = *base;
      if(stored->size() != expected->size())
      {
        EXPECT_TRUE(false) << "wrong array length, expected " <<
          expected->size() << " got " << stored->size();
        return *this;
      }

      for (auto&& [e, s] : utils::make_iterators(expected.wrapper(), stored.wrapper()))
      {
        auto vc = value_checker{ s };
        eval::on_value(e, utils::visitor{
          [&](dummy) noexcept
          {
            vc.verify();
          },
          [&](auto val) noexcept
          {
            vc.verify(val);
          } });
      }

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
    using symtab = std::unordered_map<string_t, const ir::function*>;

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
      auto end = cfg.end();
      EXPECT_NE(it, end);

      for (; it != end; ++it)
      {
        auto mod = *it;
        auto modName = mod->raw_name();
        auto res = m_st.emplace(modName, mod);
        EXPECT_TRUE(res.second);
      }
    }

    template <testable... Args>
    source_tester& test(string_t input, testable auto expected, Args... args) noexcept
    {
      utils::unused(args...);
      auto fn = find_fn(input, sizeof...(args));
      if (!fn) return *this;

      auto&& ev = m_core.ir_evaluator();
      ev.enter(*fn);
      ( ..., ev.add_arg(detail::to_value(args)) );
      ev.evaluate_current();

      value_checker{ ev.result() }.verify(expected);
      return *this;
    }

  private:
    const ir::function* find_fn(string_t name, ir::function::size_type paramCount) noexcept
    {
      EXPECT_FALSE(name.empty());
      auto nameParts = utils::split(name, "."sv);
      auto beg = nameParts.begin();
      auto end = nameParts.end();

      auto modName = *beg;
      auto modIt = m_st.find(modName);
      if (modIt == m_st.end())
      {
        EXPECT_TRUE(false) << "No module with name " << modName;
        return {};
      }
      auto fn = modIt->second;
      ++beg;

      for (; beg != end; ++beg)
      {
        auto fName = *beg;
        fn = fn->lookup(fName);
        if (!fn)
        {
          EXPECT_TRUE(false) << "No function with name " << fName;
          return {};
        }
      }

      if (const auto parCnt = fn->param_count(); parCnt != paramCount)
      {
        EXPECT_EQ(parCnt, paramCount);
        return {};
      }

      return fn;
    }

  private:
    feedback m_fb;
    core m_core;
    symtab m_st;
  };

}