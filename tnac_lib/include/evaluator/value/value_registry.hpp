//
// Value registry
//

#pragma once
#include "evaluator/value/value.hpp"

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
    using stored_val_t = underlying_val;
    using entity_vals  = std::unordered_map<entity_id, stored_val_t>;

    using val_array    = array_type::value_type;
    using ref_arr      = detail::ref_counted<val_array>;
    using array_store  = std::unordered_map<entity_id, ref_arr>;
    using size_type    = val_array::size_type;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(registry);

    registry() noexcept = default;
    ~registry() noexcept = default;

  public:
    //
    // Registers a value for a specific entity (e.g., a binary expression)
    //
    template <detail::expr_result T>
    value_type register_entity(entity_id id, T val) noexcept
    {
      auto&& valStore = m_entityValues[id];
      valStore = std::move(val);
      return { &std::get<T>(valStore) };
    }

    //
    // Destroys a value for a specific entity
    //
    value_type erase(entity_id id) noexcept
    {
      m_entityValues.erase(id);
      return {};
    }

    //
    // Resets the stored result value
    //
    value_type reset_result() noexcept
    {
      m_result = {};
      return evaluation_result();
    }

    //
    // Returns the last evaluated value
    //
    value_type evaluation_result() const noexcept
    {
      return m_result;
    }

  private:
    value_type m_result;
    entity_vals m_entityValues;
    array_store m_arrays;
  };
}