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

}