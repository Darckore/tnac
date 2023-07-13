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
    using stored_val_t = underlying_val;
    using entity_vals  = std::unordered_map<entity_id, stored_val_t>;

    using val_array    = array_type::value_type;
    using ref_arr      = detail::ref_counted<val_array>;
    using array_store  = std::unordered_map<entity_id, ref_arr>;
    using size_type    = val_array::size_type;

    using tmp_val   = temporary;
    using val_opt   = std::optional<tmp_val>;
    using tmp_store = std::queue<tmp_val>;

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
      m_result.emplace(std::move(val));
    }

  public:
    //
    // Pushes a temporary value to the queue and updates the result
    //
    void push(detail::generic_type auto val) noexcept
    {
      if (!m_result)
      {
        update_result(std::move(val));
        return;
      }

      m_inFlight.push(std::move(*m_result));
      update_result(std::move(val));
    }

    //
    // Registers a value for a specific entity (e.g., a binary expression)
    //
    void register_entity(entity_id id, detail::generic_type auto val) noexcept
    {
      using val_t = std::remove_cvref_t<decltype(val)>;
      auto&& valStore = m_entityValues[id];
      valStore = std::move(val);
      auto tmp = value{ &std::get<val_t>(valStore) };
      utils::unused(tmp);
    }

    //
    // Returns the last evaluated value
    //
    value_type evaluation_result() const noexcept
    {
      if (!m_result)
        return {};

      return *(*m_result);
    }

  private:
    val_opt m_result;
    tmp_store m_inFlight;

    entity_vals m_entityValues;
    array_store m_arrays;
    ent_id_list m_pendingCleanup;
  };
}