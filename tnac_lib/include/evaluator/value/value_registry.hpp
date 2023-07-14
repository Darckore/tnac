//
// Value registry
//

#pragma once
#include "evaluator/value/value.hpp"
#include "evaluator/type_support/traits.hpp"

namespace tnac::eval
{
  namespace detail
  {
    template <typename T>
    class ref_counted
    {
    public:
      using ref_count  = std::size_t;
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

      void ref() noexcept
      {
        ++m_ref;
      }

      void unref() noexcept
      {
        if (m_ref)
          --m_ref;
      }

    private:
      value_type m_value;
      ref_count m_ref{};
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
    using ent_id_list  = std::vector<entity_id>;
    using entity_vals  = std::unordered_map<entity_id, temporary>;

    using val_array    = array_type::value_type;
    using ref_arr      = detail::ref_counted<val_array>;
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
      m_result = std::move(val);
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
    // Decreases the ref count for an array
    //
    void unref_array(array_type& arr) noexcept
    {
      auto stored = m_arrays.find(arr.id());
      if (stored == m_arrays.end())
        return;

      auto&& controlBlock = stored->second;
      controlBlock.unref();
    }

    //
    // If the underlying type is a reference counted one, decreases the ref count
    //
    void unref(value val) noexcept
    {
      if (auto arr = val.try_get<array_type>())
        unref_array(*arr);
    }

  public:
    //
    // Pushes a temporary value to the stack and updates the result
    //
    void push(detail::generic_type auto val) noexcept
    {
      m_inFlight.push(tmp_val{ val });
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
    ent_id_list m_pendingCleanup;
  };
}