//
// Value
//

#pragma once

namespace tnac::eval
{
  struct invalid_val_t {};

  namespace detail
  {
    template <typename T>
    concept expr_result = is_any_v<T, int_type, float_type>;
  }

  enum class type_id : std::uint8_t
  {
    Invalid,
    Int,
    Float
  };

  namespace detail
  {
    template <typename T> struct id_from_type;
    template <type_id ID> struct type_from_id;

    template <>
    struct id_from_type<int_type>
    {
      static constexpr auto value = type_id::Int;
    };
    template <>
    struct type_from_id<type_id::Int>
    {
      using type = int_type;
    };

    template <>
    struct id_from_type<float_type>
    {
      static constexpr auto value = type_id::Float;
    };
    template <>
    struct type_from_id<type_id::Float>
    {
      using type = float_type;
    };
  }

  template <detail::expr_result T>
  constexpr auto id_from_type = detail::id_from_type<T>::value;

  template <type_id TI>
  using type_from_id = detail::type_from_id<TI>::type;


  //
  // Represents a value used for evaluation
  //
  class value
  {
  public:
    using value_type = std::uintptr_t;
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

    explicit operator bool() const noexcept;

  public:
    type_id id() const noexcept;
    value_type raw_value() const noexcept;

    template <detail::expr_result T>
    T get() const noexcept
    {
      return *std::bit_cast<T*>(raw_value());
    }

    template <type_id TI>
    auto get() const noexcept
    {
      return get<type_from_id<TI>>();
    }

    template <detail::expr_result T>
    value_opt<T> try_get() const noexcept
    {
      auto tv = split(m_val);
      if (!tv.val || tv.id != id_from_type<T>)
        return {};

      return get<T>();
    }

    template <type_id TI>
    auto try_get() const noexcept
    {
      return try_get<type_from_id<TI>>();
    }


  private:
    value_type m_val{};
  };

  template <typename F>
  auto on_value(value val, F&& func) noexcept
  {
    using enum type_id;
    switch (val.id())
    {
    case Int:
      return func(val.get<int_type>());

    case Float:
      return func(val.get<float_type>());

    default:
      return func(invalid_val_t{});
    }
  }
}