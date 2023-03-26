//
// Value
//

#pragma once

namespace tnac::eval
{
  namespace detail
  {
    template <typename T>
    concept expr_result = is_any_v<T, int_type, float_type>;

    enum class type_id : std::uint8_t
    {
      Invalid,
      Int,
      Float
    };
  }

  //
  // Represents a value used for evaluation
  //
  class value
  {
  public:
    using value_type = std::uintptr_t;
    using type_id = detail::type_id;
    using input_ptr = const void*;
    
    template <detail::expr_result T>
    using value_opt = std::optional<T>;

  private:
    static value_type make(input_ptr ptr, type_id id) noexcept;

    struct id_val_pair
    {
      value_type val{};
      type_id id{};
    };

    static id_val_pair split(value_type val) noexcept;

  public:
    CLASS_SPECIALS_ALL(value);

    value(input_ptr val, type_id id) noexcept;

  public:
    type_id id() const noexcept;
    value_type raw_value() const noexcept;

  private:
    value_type m_val{};
  };
}