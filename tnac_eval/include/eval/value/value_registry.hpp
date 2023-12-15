//
// Value registry
//

#pragma once
#include "eval/value/value.hpp"
#include "eval/types/traits.hpp"

namespace tnac::eval
{
  //
  // Stores instances of values used in evaluations
  //
  class registry final
  {
  public:
    using entity_vals  = std::unordered_map<entity_id, stored_value>;

    using val_array    = array_type::value_type;
    using ref_arr      = detail::ref_counted<val_array>;
    using arr_wrapper  = detail::rt_wrapper<val_array>;
    using array_store  = std::unordered_map<entity_id, ref_arr>;
    using size_type    = val_array::size_type;

    using tmp_val   = stored_value;
    using tmp_store = std::stack<tmp_val>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(registry);

    ~registry() noexcept;
    registry() noexcept;

  private:
    //
    // Updates the last result
    //
    void update_result(detail::expr_result auto val) noexcept
    {
      unref(*m_result);
      m_result = std::move(val);
      ref(*m_result);
    }

    //
    // Fetches the next value from the temporary storage
    // If the storage is empty, gets the last result
    //
    tmp_val fetch() noexcept;

    //
    // Finds an array object by id
    //
    ref_arr* get_array(entity_id id) noexcept;

    //
    // Decreases the ref count for an array
    //
    void unref_array(array_type arr) noexcept;

    //
    // If the underlying type is a reference counted one, decreases the ref count
    //
    void unref(value val) noexcept;

    //
    // Increases the ref count of an array
    //
    void ref_array(array_type arr) noexcept;

    //
    // If the underlying type is a reference counted one, increases the ref count
    //
    void ref(value val) noexcept;

    //
    // Creates a new array
    //
    arr_wrapper allocate_array(size_type prealloc) noexcept;

    //
    // Unreferences components of an array
    //
    void unref_subarrays(const val_array& arr) noexcept;

    //
    // Goes through the array collection and clears the empty ones
    //
    void cleanup_arrays() noexcept;

  public:
    //
    // Locks an array to prevent it from being removed or reused while its ref count is 0
    //
    void lock(array_type arr) noexcept;

    //
    // Unlocks an array and makes it available for removal or reuse
    //
    void unlock(array_type arr) noexcept;

  public:
    //
    // Allocates an array object and returns a reference to it
    //
    arr_wrapper make_array(size_type prealloc) noexcept;

    //
    // Registers an array from a ref counted wrapper
    //
    void push_array(arr_wrapper aw) noexcept;

    //
    // Pushes a temporary value to the stack and updates the result
    //
    void push(detail::expr_result auto val) noexcept
    {
      auto tmp = tmp_val{ val };
      ref(*tmp);
      m_inFlight.push(std::move(tmp));
      update_result(std::move(val));
    }

    //
    // Extracts the next value from the stack and returns it
    //
    tmp_val consume() noexcept;

    //
    // Registers a value for a specific entity (e.g., a variable)
    //
    void register_entity(entity_id id, detail::expr_result auto val) noexcept
    {
      auto&& stored = m_entityValues[id];
      unref(*stored);
      stored = std::move(val);
      ref(*stored);
    }

    //
    // Retrueves a stored entity value
    //
    value value_for(entity_id id) noexcept;

    //
    // Returns the last evaluated value
    //
    value evaluation_result() const noexcept;

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
    concept lockable = expr_result<T> &&
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