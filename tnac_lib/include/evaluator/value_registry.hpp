//
// Value registry
//

#pragma once
#include "evaluator/value.hpp"

namespace tnac::eval
{
  //
  // Stores instances of values used in evaluations
  //
  class registry
  {
  public:
    using value_type = value;
    using int_type   = tnac::int_type;
    using float_type = tnac::float_type;

    template <detail::expr_result T>
    using typed_store = std::unordered_set<T>;

    using entity_id = std::uintptr_t;

    using stored_val_t = std::variant<
      int_type,
      float_type
    >;

    using entity_vals  = std::unordered_map<entity_id, stored_val_t>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(registry);

    registry() noexcept;
    ~registry() noexcept;

  private:
    //
    // Updates the stored result value with the most recently registered one
    //
    template <detail::expr_result T>
    void update_result(T value) noexcept
    {
      auto&& resStore = m_entityValues[entity_id{}];
      resStore = value;
      m_result = { &std::get<T>(resStore), eval::id_from_type<T> };
    }

    //
    // Registers a value of the given type or returns an existing cached one
    //
    template <detail::expr_result T>
    const T& register_val(T value, typed_store<T>& store) noexcept
    {
      update_result(value);
      return *(store.emplace(value).first);
    }

    //
    // Stores a value for the given entity and returns a reference to it
    //
    template <detail::expr_result T>
    const T& register_val(entity_id id, T value) noexcept
    {
      update_result(value);
      auto insertIt = m_entityValues.insert_or_assign(id, value).first;
      return std::get<T>(insertIt->second);
    }

  public:
    //
    // Registers an integer
    //
    value_type register_literal(int_type val) noexcept;

    //
    // Registers a float
    //
    value_type register_literal(float_type val) noexcept;

    //
    // Registers an int value for a specific entity (e.g., a binary expression)
    //
    value_type register_entity(entity_id id, int_type val) noexcept;

    //
    // Registers a float value for a specific entity (e.g., a binary expression)
    //
    value_type register_entity(entity_id id, float_type val) noexcept;

    //
    // Resets the stored result value
    //
    value_type reset_result() noexcept;

    //
    // Returns the last evaluated value
    //
    value_type evaluation_result() const noexcept;

  private:
    value_type m_result;

    typed_store<int_type> m_ints;
    typed_store<float_type> m_floats;

    entity_vals m_entityValues;
  };
}