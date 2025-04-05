#include "eval/types.hpp"
#include "eval/type_impl.hpp"

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
  }

  array_type::array_type(reference aw) noexcept :
    m_obj{ &aw }
  {
  }

  // Public members

}