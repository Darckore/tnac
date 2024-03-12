#include "eval/value/evaluator.hpp"

namespace tnac::eval
{
  // Special members

  evaluator::~evaluator() noexcept = default;

  evaluator::evaluator(registry& reg) noexcept :
    m_registry{ reg }
  {}


  // Public members

  value evaluator::make_function(entity_id ent, function_type f) noexcept
  {
    VALUE_GUARD(m_curEntity, ent);
    reg_value(f);
    return m_registry.value_for(m_curEntity);
  }

  void evaluator::visit_binary(value lhs, value rhs, val_ops op) noexcept
  {
    if (!detail::is_binary(op))
      clear_result();

    visit_value(lhs, [this, rhs, op](auto lhs) noexcept
      {
        visit_binary(std::move(lhs), std::move(rhs), op);
      });
  }

  void evaluator::visit_unary(value val, val_ops op) noexcept
  {
    if (!detail::is_unary(op))
    {
      clear_result();
      return;
    }

    visit_value(val, [this, op](auto v) noexcept
      {
        visit_unary(std::move(v), op);
      });
  }

  void evaluator::push_value(value val) noexcept
  {
    visit_value(val, [this](auto v) noexcept
      {
        reg_value(std::move(v));
      });
  }

  void evaluator::push_last() noexcept
  {
    push_value(last());
  }

  value evaluator::visit_assign(entity_id ent, value rhs) noexcept
  {
    VALUE_GUARD(m_curEntity, ent);

    visit_value(rhs, [this](auto v) noexcept
      {
        visit_assign(std::move(v));
      });

    return m_registry.value_for(m_curEntity);
  }

  void evaluator::visit_int_literal(string_t src, int base) noexcept
  {
    auto prefix = string_t::size_type{};
    if (utils::eq_any(base, 2, 16))
      prefix = 2u;
    else if (base == 8)
      prefix = 1u;

    auto begin = src.data() + prefix;
    auto end = begin + src.length();

    int_type result{};
    auto convRes = std::from_chars(begin, end, result, base);
    if (convRes.ec != std::errc{ 0 })
    {
      clear_result();
      return;
    }

    reg_value(result);
  }

  void evaluator::visit_float_literal(string_t src) noexcept
  {
    auto begin = src.data();
    auto end = begin + src.length();

    float_type result{};
    auto convRes = std::from_chars(begin, end, result);
    if (convRes.ec != std::errc{ 0 })
    {
      clear_result();
      return;
    }

    reg_value(result);
  }

  void evaluator::visit_bool_literal(bool value) noexcept
  {
    return reg_value(value);
  }

  void evaluator::visit_i() noexcept
  {
    return reg_value(complex_type{ 0, 1 });
  }

  void evaluator::visit_pi() noexcept
  {
    return reg_value(std::numbers::pi_v<float_type>);
  }

  void evaluator::visit_e() noexcept
  {
    return reg_value(std::numbers::e_v<float_type>);
  }

  stored_value evaluator::fetch_next() noexcept
  {
    return m_registry.consume();
  }

  value evaluator::last() noexcept
  {
    return m_registry.evaluation_result();
  }

  void evaluator::clear_result() noexcept
  {
    reg_value(eval::invalid_val_t{});
  }

  void evaluator::make_array(size_type count) noexcept
  {
    auto arr = m_registry.make_array(count);
    arr->resize(count);
    fill_args(*arr, count);
    m_registry.push_array(arr);
  }

  evaluator::arr_t evaluator::collect_args_locally(size_type count) noexcept
  {
    arr_t arr;
    arr.resize(count);
    fill_args(arr, count);
    return arr;
  }

}