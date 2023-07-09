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
  class registry final
  {
  public:
    using value_type = value;

    template <detail::expr_result T>
    using typed_store = std::unordered_set<T>;

    using entity_id = std::uintptr_t;

    using stored_val_t = std::variant <
      bool_type,
      int_type,
      float_type,
      complex_type,
      fraction_type,
      function_type,
      array_type
    >;

    using entity_vals  = std::unordered_map<entity_id, stored_val_t>;
    using val_array    = array_type::value_type;
    using array_store  = std::unordered_map<entity_id, val_array>;
    using size_type    = val_array::size_type;

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
      m_result = register_val(entity_id{}, std::move(value));
    }

    //
    // Registers a value of the given type or returns an existing cached one
    //
    template <detail::expr_result T>
    value_type intern(T value) noexcept
    {
      update_result(value);
      auto&& store = store_for_type<T>();
      return { &*(store.emplace(value).first), utils::type_to_id_v<T> };
    }

    //
    // Stores a value for the given entity and returns a reference to it
    //
    template <detail::expr_result T>
    value_type register_val(entity_id id, T value) noexcept
    {
      auto&& valStore = m_entityValues[id];
      valStore = std::move(value);
      return { &std::get<T>(valStore), utils::type_to_id_v<T> };
    }

  public:
    //
    // Registers and interns a literal
    //
    template <detail::expr_result T>
      requires is_any_v<T, int_type, float_type>
    value_type register_literal(T val) noexcept
    {
      return intern(val);
    }

    //
    // Registers an already interned bool value
    //
    value_type register_literal(bool val) noexcept
    {
      static const auto boolTrue  = stored_val_t{ true };
      static const auto boolFalse = stored_val_t{ false };
      auto getValue = [val]() noexcept
      {
        return val ? &std::get<bool>(boolTrue) : &std::get<bool>(boolFalse);
      };

      update_result(val);
      return { getValue(), type_id::Bool };
    }

    //
    // Registers a value for a specific entity (e.g., a binary expression)
    //
    template <detail::expr_result T>
    value_type register_entity(entity_id id, T val) noexcept
    {
      update_result(val);
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

    typed_store<int_type> m_ints;
    typed_store<float_type> m_floats;

    entity_vals m_entityValues;
    array_store m_arrays;
  };
}