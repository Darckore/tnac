//
// Value
//

#pragma once
#include "evaluator/type_support/types.hpp"

namespace tnac::semantics
{
  class function;
}

namespace tnac::eval
{
  //
  // A dummy value which doesn't correspond to any of the supported data types
  //
  struct invalid_val_t {};

  namespace detail
  {
    //
    // Defines a valid result of expression evaluation
    //
    template <typename T>
    concept expr_result = is_any_v<T, bool_type, int_type, float_type,
                                      complex_type, fraction_type, function_type, array_type>;
  }

  //
  // Type ids for every supported type
  //
  enum class type_id : std::uint8_t
  {
    Invalid,
    Bool,
    Int,
    Float,
    Complex,
    Fraction,
    Function,
    Array
  };

  using underlying_val = std::variant<bool_type, int_type, float_type,
                                      complex_type, fraction_type, function_type, array_type>;
}

TYPE_TO_ID_ASSOCIATION(tnac::eval::bool_type,     tnac::eval::type_id::Bool);
TYPE_TO_ID_ASSOCIATION(tnac::eval::int_type,      tnac::eval::type_id::Int);
TYPE_TO_ID_ASSOCIATION(tnac::eval::float_type,    tnac::eval::type_id::Float);
TYPE_TO_ID_ASSOCIATION(tnac::eval::complex_type,  tnac::eval::type_id::Complex);
TYPE_TO_ID_ASSOCIATION(tnac::eval::fraction_type, tnac::eval::type_id::Fraction);
TYPE_TO_ID_ASSOCIATION(tnac::eval::function_type, tnac::eval::type_id::Function);
TYPE_TO_ID_ASSOCIATION(tnac::eval::array_type,    tnac::eval::type_id::Array);

namespace tnac::eval
{
  //
  // Represents a value used for evaluation
  // 
  // This is basically an opaque token corresponding to a value stored elsewhere
  // The idetifier is converted from a pointer to the target value and combined
  // with the appropriate type id
  // 
  // Since user mode doesn't use the most significant byte of the address, we can
  // use it to store any information we can jam into 8 bits
  // 
  // So, the token structure is like this:
  // -------- --------------------------
  // |  id  ||   address in memory     |
  // -------- --------------------------
  //
  class value final
  {
  public:
    using value_type = std::uintptr_t;
    using input_ptr = const void*;

  private:
    template <detail::expr_result T>
    using valptr = const T*;

  private:
    //
    // Makes the value token from a pointer to the actual stored value
    // and type id
    //
    static value_type make(input_ptr ptr, type_id id) noexcept;

    //
    // Used to extract value token parts
    //
    struct id_val_pair
    {
      value_type val{};
      type_id id{};
    };

    //
    // Splits the token into a type id and an address
    //
    static id_val_pair split(value_type val) noexcept;

  public:
    CLASS_SPECIALS_ALL(value);

    template <detail::expr_result T>
    value(valptr<T> val) noexcept :
      m_val{ make(val, utils::type_to_id_v<T>) }
    {}

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
    // Returns the token with its type id stripped
    //
    value_type raw() const noexcept;

    //
    // Unconditionally casts data at the value address to the specified type
    // The caller must ensure that the T type is correct by checking the type id
    // 
    // E.g.
    //  if(val.id() == id_from_type<cast_to_type>)
    //  {
    //    auto v = val.get<cast_to_type>();
    //  }
    //
    template <detail::expr_result T>
    decltype(auto) get() const noexcept
    {
      return *reinterpret_cast<T*>(raw());
    }

    //
    // Unconditionally casts data at the value address to the type
    // which corresponds to the specified type id
    // The caller must ensure that the actual type id is the same as the one specified
    //
    template <type_id TI>
    decltype(auto) get() const noexcept
    {
      return get<utils::id_to_type_t<TI>>();
    }

    //
    // Attempts to cast data at the value address to the specified type
    // Returns an empty std::optional<T> on failure
    //
    template <detail::expr_result T>
    auto try_get() const noexcept -> decltype(&get<T>())
    {
      auto tv = split(m_val);
      if (!tv.val || tv.id != utils::type_to_id_v<T>)
        return {};

      return &get<T>();
    }

    //
    // Attempts to cast data at the value address to the type
    // which corresponds to the specified type id
    // Returns an empty std::optional<T> on failure
    //
    template <type_id TI>
    auto try_get() const noexcept
    {
      return try_get<utils::id_to_type_t<TI>>();
    }

  private:
    value_type m_val{};
  };

  inline auto get_id(const value& val) noexcept
  {
    return val.id();
  }

  //
  // Executes the provided callable object and passes it the value converted
  // to the type actually stored at the memory location referred to by the
  // value token passed as the first parameter
  //
  template <typename F>
  auto on_value(value val, F&& func) noexcept
  {
    using enum type_id;
    switch (val.id())
    {
    case Bool:
      return func(val.get<bool_type>());

    case Int:
      return func(val.get<int_type>());

    case Float:
      return func(val.get<float_type>());

    case Complex:
      return func(val.get<complex_type>());

    case Fraction:
      return func(val.get<fraction_type>());

    case Function:
      return func(val.get<function_type>());

    case Array:
      return func(val.get<array_type>());

    default:
      return func(invalid_val_t{});
    }
  }


  //
  // Stores a temporary value
  //
  class temporary final
  {
  public:
    CLASS_SPECIALS_NOCOPY_CUSTOM(temporary);

    temporary() = default;

    template <detail::expr_result ValueType>
    explicit temporary(ValueType raw) noexcept :
      m_raw{ std::move(raw) },
      m_value{ &std::get<ValueType>(m_raw) }
    {}

    value operator*() const noexcept
    {
      return m_value;
    }

  private:
    underlying_val m_raw{};
    value m_value{};
  };
}