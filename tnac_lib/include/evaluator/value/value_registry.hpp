//
// Value registry
//

#pragma once
#include "evaluator/value/value.hpp"

namespace tnac::eval
{
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
    using array_store  = std::unordered_map<entity_id, val_array>;
    using size_type    = val_array::size_type;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(registry);

    registry() noexcept = default;
    ~registry() noexcept = default;

  private:
    //
    // Stores a value for the given entity and returns a reference to it
    //
    template <detail::expr_result T>
    value_type register_val(entity_id id, T value) noexcept
    {
      auto&& valStore = m_entityValues[id];
      valStore = std::move(value);
      return { &std::get<T>(valStore) };
    }

  public:
    //
    // Registers a value for a specific entity (e.g., a binary expression)
    //
    template <detail::expr_result T>
    value_type register_entity(entity_id id, T val) noexcept
    {
      return id != entity_id{} ? register_val(id, std::move(val)) : evaluation_result();
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
    // Creates a new array
    //
    val_array& allocate_array(entity_id id, size_type prealloc) noexcept
    {
      UTILS_ASSERT(id != entity_id{});
      auto [newElem, inserted] = m_arrays.try_emplace(id, val_array{});
      auto&& res = newElem->second;
      
      if (!inserted)
        res.clear();

      res.reserve(prealloc);
      return res;
    }

    //
    // Destroys an array allocated previously
    //
    void release_array(entity_id id) noexcept
    {
      m_arrays.erase(id);
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