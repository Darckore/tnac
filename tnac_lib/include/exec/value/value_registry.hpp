//
// Value registry
//

#pragma once
#include "exec/value/value.hpp"
#include "type_support/traits.hpp"

namespace tnac::eval
{
  namespace detail
  {
    //
    // A reference counted type wrapper
    //
    template <typename T>
    class ref_counted
    {
    public:
      using rc_type    = std::size_t;
      using value_type = T;

    public:
      CLASS_SPECIALS_NOCOPY(ref_counted);

      ref_counted(value_type val) noexcept :
        m_value{ std::move(val) }
      {}

      const value_type& value() const noexcept
      {
        return m_value;
      }
      value_type& value() noexcept
      {
        return FROM_CONST(value);
      }

      void ref() noexcept
      {
        ++m_ref;
      }
      void unref() noexcept
      {
        if (m_ref)
          --m_ref;
      }
      rc_type ref_count() const noexcept
      {
        return m_ref;
      }

    private:
      value_type m_value;
      rc_type m_ref{};
    };


    //
    // A wrapper for returning reference counted types
    //
    template <typename T>
    class rt_wrapper final
    {
    public:
      using value_type = T;
      using id_type    = std::uintptr_t;

    public:
      CLASS_SPECIALS_NODEFAULT(rt_wrapper);

      rt_wrapper(value_type& val, id_type idx) noexcept :
        m_val{ &val },
        m_id{ idx }
      {}

    public:
      const value_type* operator->() const noexcept
      {
        return m_val;
      }
      value_type* operator->() noexcept
      {
        return FROM_CONST(operator->);
      }

      const value_type& operator*() const noexcept
      {
        return *m_val;
      }
      value_type& operator*() noexcept
      {
        return FROM_CONST(operator*);
      }

      auto id() const noexcept
      {
        return m_id;
      }

    private:
      value_type* m_val{};
      id_type m_id{};
    };

  }

  //
  // Stores instances of values used in evaluations
  //
  class registry final
  {
  public:
    using value_type   = value;
    using entity_id    = std::uintptr_t;
    using entity_vals  = std::unordered_map<entity_id, temporary>;

    using val_array    = array_type::value_type;
    using ref_arr      = detail::ref_counted<val_array>;
    using arr_wrapper  = detail::rt_wrapper<val_array>;
    using array_store  = std::unordered_map<entity_id, ref_arr>;
    using size_type    = val_array::size_type;

    using tmp_val   = temporary;
    using tmp_store = std::stack<tmp_val>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(registry);

    registry() noexcept = default;
    ~registry() noexcept = default;

  private:
    //
    // Updates the last result
    //
    void update_result(detail::generic_type auto val) noexcept
    {
      unref(*m_result);
      m_result = std::move(val);
      ref(*m_result);
    }

    //
    // Fetches the next value from the temporary storage
    // If the storage is empty, gets the last result
    //
    tmp_val fetch() noexcept
    {
      tmp_val res;
      if (!m_inFlight.empty())
      {
        res = std::move(m_inFlight.top());
        m_inFlight.pop();
      }

      return res;
    }

    //
    // Finds an array object by id
    //
    ref_arr* get_array(entity_id id) noexcept
    {
      auto stored = m_arrays.find(id);
      if (stored == m_arrays.end())
        return {};

      return &stored->second;
    }

    //
    // Decreases the ref count for an array
    //
    void unref_array(array_type arr) noexcept
    {
      if (auto refArr = get_array(arr.id()))
      {
        refArr->unref();
      }
    }

    //
    // If the underlying type is a reference counted one, decreases the ref count
    //
    void unref(value val) noexcept
    {
      on_value(val, utils::visitor
      {
        [this](auto&&) noexcept {},
        [this](array_type& arr) { unref_array(arr); }
      });
    }

    //
    // Increases the ref count of an array
    //
    void ref_array(array_type arr) noexcept
    {
      if (auto refArr = get_array(arr.id()))
      {
        refArr->ref();
      }
    }

    //
    // If the underlying type is a reference counted one, increases the ref count
    //
    void ref(value val) noexcept
    {
      on_value(val, utils::visitor
      {
        [this](auto&&) noexcept {},
        [this](array_type& arr) { ref_array(arr); }
      });
    }

    //
    // Creates a new array
    //
    auto allocate_array(size_type prealloc) noexcept
    {
      auto newArr = val_array{};
      newArr.reserve(prealloc);

      auto [inserted, ok] = m_arrays.emplace(m_arrayId, ref_arr{ std::move(newArr) });
      UTILS_ASSERT(ok); // This id is not taken yet

      auto&& result = inserted->second;
      return arr_wrapper{ result.value(), m_arrayId++ };
    }

    //
    // Unreferences components of an array
    //
    void unref_subarrays(const val_array& arr) noexcept
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

    //
    // Goes through the array collection and clears the empty ones
    //
    void cleanup_arrays() noexcept
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

  public:
    //
    // Locks an array to prevent it from being removed or reused while its ref count is 0
    //
    void lock(array_type arr) noexcept
    {
      ref_array(arr);
    }

    //
    // Unlocks an array and makes it available for removal or reuse
    //
    void unlock(array_type arr) noexcept
    {
      unref_array(arr);
    }

  public:
    //
    // Allocates an array object and returns a reference to it
    //
    auto make_array(size_type prealloc) noexcept
    {
      cleanup_arrays();
      return allocate_array(prealloc);
    }

    //
    // Registers an array from a ref counted wrapper
    //
    void push_array(arr_wrapper aw) noexcept
    {
      auto&& arr = *aw;
      for (auto&& elem : arr)
      {
        ref(*elem);
      }
      auto val = array_type{ arr, aw.id() };
      push(val);
    }

    //
    // Pushes a temporary value to the stack and updates the result
    //
    void push(detail::generic_type auto val) noexcept
    {
      auto tmp = tmp_val{ val };
      ref(*tmp);
      m_inFlight.push(std::move(tmp));
      update_result(std::move(val));
    }

    //
    // Extracts the next value from the stack and returns it
    //
    tmp_val consume() noexcept
    {
      auto res = fetch();
      unref(*res);
      return res;
    }

    //
    // Registers a value for a specific entity (e.g., a variable)
    //
    void register_entity(entity_id id, detail::generic_type auto val) noexcept
    {
      auto&& stored = m_entityValues[id];
      unref(*stored);
      stored = std::move(val);
      ref(*stored);
    }

    //
    // Retrueves a stored entity value
    //
    value value_for(entity_id id) noexcept
    {
      auto item = m_entityValues.find(id);
      if (item == m_entityValues.end())
        return {};

      return *(item->second);
    }

    //
    // Returns the last evaluated value
    //
    value_type evaluation_result() const noexcept
    {
      return *m_result;
    }

  private:
    tmp_val m_result;
    tmp_store m_inFlight;

    entity_vals m_entityValues;
    array_store m_arrays;
    size_type m_arrayId{};
  };


  namespace detail
  {
    template <typename T>
    concept lockable = generic_type<T> &&
      requires(T t, registry& reg)
    {
      { reg.lock(t) }   -> std::same_as<void>;
      { reg.unlock(t) } -> std::same_as<void>;
    };
  }

  //
  // A RAII wrapper which allows locking and unlocking ref counted types
  //
  template <detail::lockable T>
  class value_lock final
  {
  public:
    using value_type = T;

  public:
    CLASS_SPECIALS_NONE(value_lock);

    value_lock(const value_type& val, registry& reg) noexcept :
      m_reg{ reg },
      m_value{ val }
    {
      m_reg.lock(m_value);
    }

    ~value_lock() noexcept
    {
      m_reg.unlock(m_value);
    }

  private:
    registry& m_reg;
    const value_type& m_value;
  };

}