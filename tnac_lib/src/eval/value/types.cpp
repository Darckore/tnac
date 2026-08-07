#include "eval/value/types.hpp"
#include "eval/value/type_impl.hpp"

// functions
namespace tnac::eval
{
  // Special members

  function_type::~function_type() noexcept = default;

  function_type::function_type(reference func) noexcept :
    rc_base{ rc_base::empty_tag{} },
    m_func{ &func }
  {
  }

  function_type::function_type(const function_type&) noexcept = default;
  function_type& function_type::operator=(const function_type&) noexcept = default;
  function_type::function_type(function_type&&) noexcept = default;
  function_type& function_type::operator=(function_type&&) noexcept = default;

  bool function_type::operator==(const function_type& other) const noexcept
  {
    return m_func == other.m_func && get() == other.get();
  }

  void function_type::attach_closure(array_data& data) noexcept
  {
    reinit(&data);
  }

  bool function_type::is_closure() const noexcept
  {
    return static_cast<bool>(get());
  }

  const array_data& function_type::closure_data() const noexcept
  {
    UTILS_ASSERT(is_closure());
    return *get();
  }
  array_data& function_type::closure_data() noexcept
  {
    return FROM_CONST(closure_data);
  }


  // Public members

  function_type::const_pointer function_type::operator->() const noexcept
  {
    return m_func;
  }
  function_type::pointer function_type::operator->() noexcept
  {
    return FROM_CONST(operator->);
  }

  function_type::const_reference function_type::operator*() const noexcept
  {
    return *m_func;
  }
  function_type::reference function_type::operator*() noexcept
  {
    return FROM_CONST(operator*);
  }
}


// arrays
namespace tnac::eval
{
  // Special members

  array_type::~array_type() noexcept = default;

  array_type::array_type(reference aw) noexcept :
    rc_base{ aw }
  {
  }

  array_type::array_type(const array_type&) noexcept = default;
  array_type& array_type::operator=(const array_type&) noexcept = default;
  array_type::array_type(array_type&&) noexcept = default;
  array_type& array_type::operator=(array_type&&) noexcept = default;


  // Public members

  const array_wrapper& array_type::wrapper() const noexcept
  {
    return *operator->();
  }

  array_wrapper& array_type::wrapper() noexcept
  {
    return FROM_CONST(wrapper);
  }
}