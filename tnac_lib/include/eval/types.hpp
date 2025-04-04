//
// Supported types
//

#pragma once

#define TNAC_TYPES invalid_val_t,\
bool_type,\
int_type,\
float_type,\
complex_type,\
fraction_type,\
function_type,\
array_type

namespace tnac::eval
{
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
}

namespace tnac::ir
{
  class function;
}

namespace tnac::eval
{
  using bool_type = bool;
  using int_type = std::intmax_t;
  using float_type = double;
  using complex_type = std::complex<float_type>;
  using fraction_type = utils::ratio<int_type>;

  class function_type;
  class array_type;

  //
  // A dummy value which doesn't correspond to any of the supported data types
  //
  struct invalid_val_t {};

  //
  // Defines a valid result of expression evaluation
  //
  template <typename T>
  concept expr_result = utils::any_same_as<T, TNAC_TYPES>;
}

namespace tnac::eval
{
  //
  // Function wrapper
  // Represents the function value type
  //
  class function_type final
  {
  public:
    using value_type = ir::function;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;

  public:
    CLASS_SPECIALS_NODEFAULT(function_type);

    ~function_type() noexcept = default;

    function_type(reference func) noexcept :
      m_func{ &func }
    {
    }

    bool operator==(const function_type& other) const noexcept = default;

  public:
    const_pointer operator->() const noexcept
    {
      return m_func;
    }
    pointer operator->() noexcept
    {
      return FROM_CONST(operator->);
    }

    const_reference operator*() const noexcept
    {
      return *m_func;
    }
    reference operator*() noexcept
    {
      return FROM_CONST(operator*);
    }

  private:
    pointer m_func{};
  };
}

namespace tnac::eval
{
  //
  // Array type
  // Represents an arbitrary-sized array of values
  //
  class array_type final
  {
  public:

  public:
    CLASS_SPECIALS_ALL(array_type);

    ~array_type() noexcept = default;
  };
}

TYPE_TO_ID_ASSOCIATION(tnac::eval::invalid_val_t, tnac::eval::type_id::Invalid);
TYPE_TO_ID_ASSOCIATION(tnac::eval::bool_type,     tnac::eval::type_id::Bool);
TYPE_TO_ID_ASSOCIATION(tnac::eval::int_type,      tnac::eval::type_id::Int);
TYPE_TO_ID_ASSOCIATION(tnac::eval::float_type,    tnac::eval::type_id::Float);
TYPE_TO_ID_ASSOCIATION(tnac::eval::complex_type,  tnac::eval::type_id::Complex);
TYPE_TO_ID_ASSOCIATION(tnac::eval::fraction_type, tnac::eval::type_id::Fraction);
TYPE_TO_ID_ASSOCIATION(tnac::eval::function_type, tnac::eval::type_id::Function);
TYPE_TO_ID_ASSOCIATION(tnac::eval::array_type,    tnac::eval::type_id::Array);