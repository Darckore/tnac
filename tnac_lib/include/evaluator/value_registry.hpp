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

    registry() noexcept = default;
    ~registry() noexcept = default;

  private:
    template <detail::expr_result T>
    typed_store<T>& store_for_type() noexcept;

    //
    // Gets the store for interned integers
    //
    template<>
    typed_store<int_type>& store_for_type() noexcept
    {
      return m_ints;
    }

    //
    // Gets the store for interned floats
    //
    template<>
    typed_store<float_type>& store_for_type() noexcept
    {
      return m_floats;
    }

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
    value_type intern(T value) noexcept
    {
      update_result(value);
      auto&& store = store_for_type<T>();
      return { &*(store.emplace(value).first), eval::id_from_type<T> };
    }

    //
    // Stores a value for the given entity and returns a reference to it
    //
    template <detail::expr_result T>
    value_type register_val(entity_id id, T value) noexcept
    {
      update_result(value);
      auto&& valStore = m_entityValues[id];
      valStore = value;
      return { &std::get<T>(valStore), eval::id_from_type<T> };
    }

  public:
    //
    // Registers and interns a literal
    //
    template <detail::expr_result T>
    value_type register_literal(T val) noexcept
    {
      return intern(val);
    }

    //
    // Registers a value for a specific entity (e.g., a binary expression)
    //
    template <detail::expr_result T>
    value_type register_entity(entity_id id, T val) noexcept
    {
      return register_val(id, val);
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

    typed_store<int_type> m_ints;
    typed_store<float_type> m_floats;

    entity_vals m_entityValues;
  };
}