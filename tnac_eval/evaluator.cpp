#if 0

namespace tnac::eval // Binary ops
{

  void evaluator::equal(array_type lhs, array_type rhs, bool compareForEquality) noexcept
  {
    auto&& l = *lhs;
    auto&& r = *rhs;

    if (compareForEquality && &l == &r)
    {
      reg_value(true);
      return;
    }
    if (l.size() != r.size())
    {
      reg_value(!compareForEquality);
      return;
    }

    for (auto&& [le, re] : utils::make_iterators(l, r))
    {
      visit_binary(*le, *re, val_ops::Equal);
      const auto cmp = to_bool(*fetch_next());
      if (!cmp)
      {
        reg_value(compareForEquality ? false : true);
        return;
      }
    }

    reg_value(compareForEquality);
  }

  void evaluator::less(array_type lhs, array_type rhs) noexcept
  {
    auto&& l = *lhs;
    auto&& r = *rhs;
    if (&l == &r)
    {
      reg_value(false);
      return;
    }

    for (auto&& [lv, rv] : utils::make_iterators(l, r))
    {
      visit_binary(*lv, *rv, val_ops::RelLess);
      const auto cmp = to_bool(*fetch_next());
      if (cmp)
      {
        reg_value(true);
        return;
      }
    }

    reg_value(l.size() < r.size());
  }

  void evaluator::less_eq(array_type lhs, array_type rhs) noexcept
  {
    less(lhs, rhs);
    if (to_bool(*fetch_next()))
    {
      reg_value(true);
      return;
    }
    equal(std::move(lhs), std::move(rhs), true);
    reg_value(to_bool(*fetch_next()));
  }

  void evaluator::greater(array_type lhs, array_type rhs) noexcept
  {
    auto&& l = *lhs;
    auto&& r = *rhs;
    if (&l == &r)
    {
      reg_value(false);
      return;
    }

    for (auto&& [lv, rv] : utils::make_iterators(l, r))
    {
      visit_binary(*lv, *rv, val_ops::RelGr);
      const auto cmp = to_bool(*fetch_next());
      if (cmp)
      {
        reg_value(true);
        return;
      }
    }

    reg_value(l.size() > r.size());
  }

  void evaluator::greater_eq(array_type lhs, array_type rhs) noexcept
  {
    greater(lhs, rhs);
    if (to_bool(*fetch_next()))
    {
      reg_value(true);
      return;
    }
    equal(std::move(lhs), std::move(rhs), true);
    reg_value(to_bool(*fetch_next()));
  }
}

namespace tnac::eval // Operations main
{
  template <>
  void evaluator::visit_unary(array_type operand, val_ops op) noexcept
  {
    utils::unused(operand, op);
    //if (utils::eq_any(op, val_ops::LogicalIs, val_ops::LogicalNot))
    //{
    //  auto toBool = get_caster<bool_type>()(std::move(operand));
    //  return visit_unary(toBool.value_or(false), op);
    //}

    //value_lock _{ operand, m_registry };
    //for (auto&& el : *operand)
    //{
    //  visit_unary(*el, op);
    //}

    //make_array(operand->size());
  }

  template <>
  void evaluator::visit_binary(array_type l, array_type r, val_ops op) noexcept
  {
    utils::unused(l, r, op);
    //if (detail::is_comparison(op))
    //{
    //  using enum val_ops;
    //  switch (op)
    //  {
    //  case RelLess:   less(std::move(l), std::move(r));         break;
    //  case RelLessEq: less_eq(std::move(l), std::move(r));      break;
    //  case RelGr:     greater(std::move(l), std::move(r));      break;
    //  case RelGrEq:   greater_eq(std::move(l), std::move(r));   break;
    //  case Equal:     equal(std::move(l), std::move(r), true);  break;
    //  case NEqual:    equal(std::move(l), std::move(r), false); break;
    //  }
    //  return;
    //}

    //static auto unitArr = []() noexcept { arr_t ret; ret.emplace_back(int_type{}); return ret; }();
    //const auto lsz = l->size();
    //if (!lsz) l = array_type{ unitArr };
    //const auto rsz = r->size();
    //if (!rsz) r = array_type{ unitArr };
    //const auto newSz = (lsz && rsz) ? lsz * rsz : std::max(lsz, rsz);

    //value_lock _l{ l, m_registry };
    //value_lock _r{ r, m_registry };

    //for (auto&& el : *l)
    //{
    //  for (auto&& er : *r)
    //  {
    //    visit_binary(*el, *er, op);
    //  }
    //}

    //make_array(newSz);
  }

  template <expr_result T> requires (!utils::same_noquals<T, array_type>)
  void evaluator::visit_binary(array_type l, T r, val_ops op) noexcept
  {
    auto rhs = to_unit_array(r);
    visit_binary(std::move(l), array_type{ rhs }, op);
  }

  template <expr_result T> requires (!utils::same_noquals<T, array_type>)
  void evaluator::visit_binary(T l, array_type r, val_ops op) noexcept
  {
    auto lhs = to_unit_array(l);
    visit_binary(array_type{ lhs }, std::move(r), op);
  }
}

#endif