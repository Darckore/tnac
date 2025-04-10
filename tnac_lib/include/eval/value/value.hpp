//
// Value
//

#pragma once
#include "eval/value/types.hpp"

namespace tnac::eval
{
  enum class val_ops : std::uint8_t;
  class array_wrapper;
  class store;

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
    // Converts an id to string
    //
    static string_t id_str(type_id id) noexcept;

    //
    // Returns the current value id as string
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
      return std::get_if<T>(&m_raw);
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

  public: // Evaluation
    //
    // Parses an integer literal value from string
    //
    static value parse_int(string_t src, int base) noexcept;

    //
    // Parses a floating point literal value from string
    //
    static value parse_float(string_t src) noexcept;

    //
    // Returns the value for pi
    //
    static value pi() noexcept;

    //
    // Returns the value for e
    //
    static value e() noexcept;

    //
    // Returns the value for i
    //
    static value i() noexcept;

    //
    // Returns the value for true
    //
    static value true_val() noexcept;

    //
    // Returns the value for false
    //
    static value false_val() noexcept;

    //
    // Returns a value for a function
    //
    static value function(ir::function& func) noexcept;

    //
    // Returns a value for an array
    //
    static value array(array_wrapper& aw) noexcept;

    //
    // Applies a unary operation to the current value and returns a new resulting one
    //
    value unary(val_ops op) const noexcept;

    //
    // Applies a binary operation to the current value and the provided value,
    // and returns a new resulting one
    //
    value binary(val_ops op, const value& rhs) const noexcept;

  private:
    bool is_array() const noexcept;

    value unary_as_array(val_ops op) const noexcept;

    value binary_as_array(val_ops op, const value& rhs) const noexcept;

    store* extract_store() const noexcept;

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