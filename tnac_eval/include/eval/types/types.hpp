//
// Supported types
//

#pragma once

namespace tnac::semantics
{
  class function;
}

namespace tnac::eval::detail
{
  //
  // A reference counted type wrapper
  //
  template <typename T>
  class ref_counted
  {
  public:
    using rc_type = std::size_t;
    using value_type = T;

  public:
    CLASS_SPECIALS_NOCOPY(ref_counted);

    ref_counted(value_type val) noexcept :
      m_value{ std::move(val) }
    {}

    const value_type& value() const noexcept
    {
      return m_value;
    }
    value_type& value() noexcept
    {
      return FROM_CONST(value);
    }

    void ref() noexcept
    {
      ++m_ref;
    }
    void unref() noexcept
    {
      if (m_ref)
        --m_ref;
    }
    rc_type ref_count() const noexcept
    {
      return m_ref;
    }

  private:
    value_type m_value;
    rc_type m_ref{};
  };


  //
  // A wrapper for returning reference counted types
  //
  template <typename T>
  class rt_wrapper final
  {
  public:
    using value_type = T;
    using id_type = std::uintptr_t;

  public:
    CLASS_SPECIALS_NODEFAULT(rt_wrapper);

    rt_wrapper(value_type& val, id_type idx) noexcept :
      m_val{ &val },
      m_id{ idx }
    {}

  public:
    const value_type* operator->() const noexcept
    {
      return m_val;
    }
    value_type* operator->() noexcept
    {
      return FROM_CONST(operator->);
    }

    const value_type& operator*() const noexcept
    {
      return *m_val;
    }
    value_type& operator*() noexcept
    {
      return FROM_CONST(operator*);
    }

    auto id() const noexcept
    {
      return m_id;
    }

  private:
    value_type* m_val{};
    id_type m_id{};
  };
}

namespace tnac::eval
{
  class value;
  class stored_value;

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
    using value_type      = std::vector<stored_value>;
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
      array_type{ underlying, ~id_type{} }
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