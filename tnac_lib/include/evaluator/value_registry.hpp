//
// Value registry
//

#pragma once
#include "evaluator/value.hpp"

namespace tnac::eval
{
  //
  // Stores instances of values
  //
  class registry
  {
  public:
    using value_type = value;
    using int_type   = tnac::int_type;
    using float_type = tnac::float_type;

    template <detail::expr_result T>
    using typed_store = std::unordered_set<T>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(registry);

    registry() noexcept;
    ~registry() noexcept;

  private:
    //
    // Registers a value of the given type or returns an existing cached one
    //
    template <detail::expr_result T>
    const T& register_val(T value, typed_store<T>& store) noexcept
    {
      auto insertResult = store.emplace(value);
      return *(insertResult.first);
    }

  public:
    //
    // Registers an integer
    //
    value_type register_int(int_type val) noexcept;

  private:
    typed_store<int_type> m_ints;
    typed_store<float_type> m_floats;
  };
}