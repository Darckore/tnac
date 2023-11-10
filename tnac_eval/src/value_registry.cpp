#include "eval/value/value_registry.hpp"

namespace tnac::eval
{
  // Special members

  registry::~registry() noexcept = default;

  registry::registry() noexcept = default;


  // Public members

  void registry::lock(array_type arr) noexcept
  {
    ref_array(arr);
  }

  void registry::unlock(array_type arr) noexcept
  {
    unref_array(arr);
  }

  registry::arr_wrapper registry::make_array(size_type prealloc) noexcept
  {
    cleanup_arrays();
    return allocate_array(prealloc);
  }

  void registry::push_array(arr_wrapper aw) noexcept
  {
    auto&& arr = *aw;
    for (auto&& elem : arr)
    {
      ref(*elem);
    }
    auto val = array_type{ arr, aw.id() };
    push(val);
  }

  registry::tmp_val registry::consume() noexcept
  {
    auto res = fetch();
    unref(*res);
    return res;
  }

  value registry::value_for(entity_id id) noexcept
  {
    auto item = m_entityValues.find(id);
    if (item == m_entityValues.end())
      return {};

    return *(item->second);
  }

  value registry::evaluation_result() const noexcept
  {
    return *m_result;
  }


  // Private members

  registry::tmp_val registry::fetch() noexcept
  {
    tmp_val res;
    if (!m_inFlight.empty())
    {
      res = std::move(m_inFlight.top());
      m_inFlight.pop();
    }

    return res;
  }

  registry::ref_arr* registry::get_array(entity_id id) noexcept
  {
    auto stored = m_arrays.find(id);
    if (stored == m_arrays.end())
      return {};

    return &stored->second;
  }

  void registry::unref_array(array_type arr) noexcept
  {
    if (auto refArr = get_array(arr.id()))
    {
      refArr->unref();
    }
  }

  void registry::unref(value val) noexcept
  {
    on_value(val, utils::visitor
      {
        [this](auto&&) noexcept {},
        [this](array_type& arr) { unref_array(arr); }
      });
  }

  void registry::ref_array(array_type arr) noexcept
  {
    if (auto refArr = get_array(arr.id()))
    {
      refArr->ref();
    }
  }

  void registry::ref(value val) noexcept
  {
    on_value(val, utils::visitor
      {
        [this](auto&&) noexcept {},
        [this](array_type& arr) { ref_array(arr); }
      });
  }

  registry::arr_wrapper registry::allocate_array(size_type prealloc) noexcept
  {
    auto newArr = val_array{};
    newArr.reserve(prealloc);

    auto [inserted, ok] = m_arrays.emplace(m_arrayId, ref_arr{ std::move(newArr) });
    UTILS_ASSERT(ok); // This id is not taken yet

    auto&& result = inserted->second;
    return arr_wrapper{ result.value(), m_arrayId++ };
  }

  void registry::unref_subarrays(const val_array& arr) noexcept
  {
    for (auto&& elem : arr)
    {
      auto val = *elem;
      unref(val);
      if (auto arrT = val.try_get<array_type>())
      {
        auto subArr = get_array(arrT->id());
        if (!subArr)
          continue;

        unref_subarrays(subArr->value());
      }
    }
  }

  void registry::cleanup_arrays() noexcept
  {
    for (auto arrIt = m_arrays.begin(); arrIt != m_arrays.end(); )
    {
      auto&& [id, arr] = *arrIt;
      if (arr.ref_count())
      {
        ++arrIt;
        continue;
      }

      unref_subarrays(arr.value());
      arrIt = m_arrays.erase(arrIt);
    }
  }

}