#include "eval/value/types.hpp"
#include "eval/value/type_impl.hpp"

// functions
namespace tnac::eval
{
  // Special members

  function_type::~function_type() noexcept = default;

  function_type::function_type(reference func) noexcept :
    m_func{ &func }
  {
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

  array_type::~array_type() noexcept
  {
    auto obj = operator->();
    if (!obj || !obj->is_last())
      return;

    auto&& list = obj->list();
    list.remove(*obj);
  }

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
}