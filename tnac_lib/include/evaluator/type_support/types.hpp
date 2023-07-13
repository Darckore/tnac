//
// Supported types
//

#pragma once

namespace tnac::semantics
{
  class function;
}

namespace tnac::eval
{
  class value;

  using bool_type     = bool;
  using int_type      = std::intmax_t;
  using float_type    = double;
  using complex_type  = std::complex<float_type>;
  using fraction_type = utils::ratio<int_type>;

  //
  // Function wrapper
  // Represents the function value type
  //
  class function_type final
  {
  public:
    using value_type = semantics::function;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;

  public:
    CLASS_SPECIALS_NODEFAULT(function_type);

    ~function_type() noexcept = default;

    function_type(reference func) noexcept :
      m_func{ &func }
    {}

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


  //
  // Array type
  // Represents an arbitrary-sized array of values
  //
  class array_type final
  {
  public:
    using value_type      = std::vector<value>;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using id_type         = std::uintptr_t;

  public:
    CLASS_SPECIALS_NODEFAULT(array_type);

    ~array_type() noexcept = default;

    array_type(const_reference underlying, id_type objId) noexcept :
      m_underlying{ &underlying },
      m_id{ objId }
    {}

    array_type(const_reference underlying) noexcept :
      array_type{ underlying, {} }
    {}

  public:
    const_pointer operator->() const noexcept
    {
      return m_underlying;
    }

    const_reference operator*() const noexcept
    {
      return *m_underlying;
    }

    id_type id() const noexcept
    {
      return m_id;
    }

  private:
    const_pointer m_underlying{};
    id_type m_id{};
  };
}