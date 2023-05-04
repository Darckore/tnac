//
// Value
//

#pragma once

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
    concept expr_result = is_any_v<T, int_type, float_type, complex_type, fraction_type>;
  }

  //
  // Used for binary expressions. Converts values to their common type
  //
  template <detail::expr_result T1, detail::expr_result T2>
  struct common_type_cast : public tnac::detail::common_type<tnac::detail::nocvref<T1>, tnac::detail::nocvref<T2>>
  {
    constexpr auto operator()(T1 v1, T2 v2) noexcept
    {
      using res_type = common_type_cast<T1, T2>::type;
      return std::pair{ static_cast<res_type>(v1), static_cast<res_type>(v2) };
    }
  };

  //
  // Type ids for every supported type
  //
  enum class type_id : std::uint8_t
  {
    Invalid,
    Int,
    Float,
    Complex,
    Fraction
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

    template <>
    struct id_from_type<complex_type>
    {
      static constexpr auto value = type_id::Complex;
    };
    template <>
    struct type_from_id<type_id::Complex>
    {
      using type = complex_type;
    };

    template <>
    struct id_from_type<fraction_type>
    {
      static constexpr auto value = type_id::Fraction;
    };
    template <>
    struct type_from_id<type_id::Fraction>
    {
      using type = fraction_type;
    };
  }

  template <detail::expr_result T>
  inline constexpr auto id_from_type = detail::id_from_type<T>::value;

  template <type_id TI>
  using type_from_id = detail::type_from_id<TI>::type;


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
    
    template <detail::expr_result T>
    using value_opt = std::optional<T>;

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

    value(input_ptr val, type_id id) noexcept;

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
    value_type raw_value() const noexcept;

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
    T get() const noexcept
    {
      return *reinterpret_cast<T*>(raw_value());
    }

    //
    // Unconditionally casts data at the value address to the type
    // which corresponds to the specified type id
    // The caller must ensure that the actual type id is the same as the one specified
    //
    template <type_id TI>
    auto get() const noexcept
    {
      return get<type_from_id<TI>>();
    }

    //
    // Attempts to cast data at the value address to the specified type
    // Returns an empty std::optional<T> on failure
    //
    template <detail::expr_result T>
    value_opt<T> try_get() const noexcept
    {
      auto tv = split(m_val);
      if (!tv.val || tv.id != id_from_type<T>)
        return {};

      return get<T>();
    }

    //
    // Attempts to cast data at the value address to the type
    // which corresponds to the specified type id
    // Returns an empty std::optional<T> on failure
    //
    template <type_id TI>
    auto try_get() const noexcept
    {
      return try_get<type_from_id<TI>>();
    }

  private:
    value_type m_val{};
  };

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
    case Int:
      return func(val.get<int_type>());

    case Float:
      return func(val.get<float_type>());

    case Complex:
      return func(val.get<complex_type>());

    case Fraction:
      return func(val.get<fraction_type>());

    default:
      return func(invalid_val_t{});
    }
  }
}