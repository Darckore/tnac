//
// Value
//

#pragma once
#include "eval/types.hpp"

namespace tnac::eval
{
  //
  // Stores a value
  //
  class value final
  {
  public:
    using enum type_id;
    using underlying_val = std::variant<TNAC_TYPES>;
    using size_type      = decltype(sizeof(0));

  public:
    CLASS_SPECIALS_ALL_CUSTOM(value);

    ~value() noexcept;

    explicit value(expr_result auto raw) noexcept :
      m_raw{ std::move(raw) }
    {
    }

    value() noexcept;

    value& operator=(expr_result auto raw) noexcept
    {
      m_raw = std::move(raw);
      return *this;
    }

    value& operator=(invalid_val_t inv) noexcept;

    //
    // Checks whether the value is valid (type id != Invalid)
    //
    explicit operator bool() const noexcept;

  public:
    //
    // Returns the value's type id
    //
    type_id id() const noexcept;

    //
    // Returns the value id as string
    //
    string_t id_str() const noexcept;

    //
    // Returns the underlying value's size in bytes
    //
    size_type size() const noexcept;

    //
    // Extracts the underlying value as the specified type
    // Use with care. Will break if the underlying value is of a wrong type
    //
    template <expr_result T>
    decltype(auto) get() const noexcept
    {
      return std::get<T>(m_raw);
    }

    //
    // Extracts the underlying value as the type which corresponds to the specified type id
    // The caller must ensure that the actual type id is the same as the one specified
    //
    template <type_id TI>
    decltype(auto) get() const noexcept
    {
      return get<utils::id_to_type_t<TI>>();
    }

    //
    // Attempts to extract the underlying value as the specified type
    // Returns a nullptr on failure
    //
    template <expr_result T>
    auto try_get() const noexcept -> decltype(&get<T>())
    {
      return &std::get_if<T>(&m_raw);
    }

    //
    // Attempts to extract the underlying value
    // as the type which corresponds to the specified type id
    // Returns a nullptr on failure
    //
    template <type_id TI>
    auto try_get() const noexcept
    {
      return try_get<utils::id_to_type_t<TI>>();
    }

  private:
    underlying_val m_raw{};
  };

  inline auto get_id(const value& val) noexcept
  {
    return val.id();
  }

  //
  // Dispatches the provided callable object depending on the stored type
  //
  template <typename F>
  auto on_value(const value& val, F&& func) noexcept
  {
    using enum type_id;
    switch (val.id())
    {
    case Bool:     return func(val.get<bool_type>());
    case Int:      return func(val.get<int_type>());
    case Float:    return func(val.get<float_type>());
    case Complex:  return func(val.get<complex_type>());
    case Fraction: return func(val.get<fraction_type>());
    case Function: return func(val.get<function_type>());
    case Array:    return func(val.get<array_type>());
    default:       return func(invalid_val_t{});
    }
  }
}