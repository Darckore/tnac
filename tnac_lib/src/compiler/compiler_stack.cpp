#include "compiler/detail/compiler_stack.hpp"

namespace tnac::detail
{
  // Special members

  compiler_stack::~compiler_stack() noexcept = default;

  compiler_stack::compiler_stack() noexcept = default;


  // Public members

  void compiler_stack::push(ir::operand op) noexcept
  {
    m_data.emplace_back(op);
  }

  void compiler_stack::push_undef() noexcept
  {
    push(eval::value{});
  }

  compiler_stack::value_type compiler_stack::top() noexcept
  {
    UTILS_ASSERT(!m_data.empty());
    return m_data.back();
  }

  void compiler_stack::pop() noexcept
  {
    if (m_data.empty()) return;
    m_data.pop_back();
  }

  void compiler_stack::drop(size_type count) noexcept
  {
    if (m_data.size() < count)
    {
      m_data.clear();
      return;
    }

    while (count--)
      pop();
  }

  compiler_stack::value_type compiler_stack::extract() noexcept
  {
    SCOPE_GUARD(pop());
    return top();
  }

  compiler_stack::value_type compiler_stack::try_extract() noexcept
  {
    if (empty())
      return eval::value{};
    return extract();
  }

  bool compiler_stack::empty() const noexcept
  {
    return m_data.empty();
  }

  compiler_stack::size_type compiler_stack::size() const noexcept
  {
    return m_data.size();
  }

  bool compiler_stack::has_values(size_type count) const noexcept
  {
    if (m_data.size() < count)
      return false;

    for (auto cnt = size_type{}; auto&& op : m_data | views::reverse)
    {
      if (cnt == count)
        break;

      if (!op.is_value())
        return false;

      ++cnt;
    }
    return true;
  }

  bool compiler_stack::has_at_least(size_type count) const noexcept
  {
    return m_data.size() >= count;
  }

  void compiler_stack::fill(ir::instruction& instr, size_type count) noexcept
  {
    walk_back(count, [&instr](auto op) noexcept
      {
        instr.add(std::move(op));
      });
  }

  //void compiler_stack::fill(arr_type& arr, size_type count) noexcept
  //{
  //  utils::unused(arr, count);
  //  //walk_back(count, [&arr](auto op) noexcept
  //  //  {
  //  //    UTILS_ASSERT(op.is_value());
  //  //    arr.emplace_back(op.get_value());
  //  //  });
  //}

  template <eval::expr_result Obj, typename Int, Int... Seq>
  void compiler_stack::instantiate(cval_array<sizeof...(Seq)>& args, utils::idx_seq<Int, Seq...>) noexcept
  {
    using type_info = eval::type_info<Obj>;
    using type_gen  = eval::type_wrapper<Obj>;
    auto instance = type_gen{}(
      eval::cast_value<utils::id_to_type_t<type_info::params[Seq]>>(args[Seq])...);

    eval::value res{};
    if (instance)
      res = *instance;
    
    push(res);
  }

  template <eval::expr_result Obj>
  void compiler_stack::instantiate(size_type argSz) noexcept
  {
    static constexpr auto max = eval::type_info<Obj>::maxArgs;
    val_array<max> args{};
    size_type idx{};
    walk_back(argSz, [&args, &idx](auto op) noexcept
      {
        UTILS_ASSERT(op.is_value());
        args[idx++] = op.get_value();
      });
    
    instantiate<Obj>(args, utils::idx_gen<max>{});
  }

  void compiler_stack::instantiate(eval::type_id type, size_type argSz) noexcept
  {
    using enum eval::type_id;
    switch (type)
    {
    case Bool:     instantiate<eval::bool_type>(argSz); break;
    case Int:      instantiate<eval::int_type>(argSz); break;
    case Float:    instantiate<eval::float_type>(argSz); break;
    case Complex:  instantiate<eval::complex_type>(argSz); break;
    case Fraction: instantiate<eval::fraction_type>(argSz); break;

    default: break;
    }
  }

  // Private members

  void compiler_stack::walk_back(size_type count, op_processor auto&& proc) noexcept
  {
    if (!count)
      return;

    UTILS_ASSERT(m_data.size() >= count);
    auto beg = std::next(m_data.begin(), m_data.size() - count);
    for (auto it = beg; it < m_data.end(); ++it)
      proc(*it);

    while (count--)
      pop();
  }
}