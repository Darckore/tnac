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
    using size_type    = val_array::size_type;
    using tmp_val      = stored_value;
    using tmp_store    = utils::stack<tmp_val>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(registry);

    ~registry() noexcept;
    registry() noexcept;

  private:
    //
    // Updates the last result
    //
    void update_result(expr_result auto val) noexcept
    {
      m_result = std::move(val);
    }

    //
    // Fetches the next value from the temporary storage
    // If the storage is empty, gets the last result
    //
    tmp_val fetch() noexcept;

  public:
    //
    // Pushes a temporary value to the stack and updates the result
    //
    void push(expr_result auto val) noexcept
    {
      auto tmp = tmp_val{ val };
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
    void register_entity(entity_id id, expr_result auto val) noexcept
    {
      auto&& stored = m_entityValues[id];
      stored = std::move(val);
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
    size_type m_arrayId{};
  };
}