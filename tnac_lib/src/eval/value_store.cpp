#include "eval/value_store.hpp"
#include "eval/value.hpp"
#include "eval/type_impl.hpp"

namespace tnac::eval
{
  // Special members

  store::~store() noexcept = default;

  store::store() noexcept = default;


  // Public members

  array_data& store::allocate_array(size_type size) noexcept
  {
    return m_arrData.emplace_back(*this, size);
  }

  array_wrapper& store::wrap(array_data& arr, size_type offset, size_type size) noexcept
  {
    return m_arrWrappers.emplace_back(arr, offset, size);
  }

  array_wrapper& store::wrap(array_data& arr, size_type offset) noexcept
  {
    return m_arrWrappers.emplace_back(arr, offset);
  }

  array_wrapper& store::wrap(array_data& arr) noexcept
  {
    return m_arrWrappers.emplace_back(arr);
  }

  array_wrapper& store::wrap(array_wrapper& aw, size_type offset, size_type size) noexcept
  {
    const auto origSize = aw.size();
    const auto origOff = aw.offset();
    if (origSize == size && origOff == offset)
      return aw;

    return wrap(aw.data(), offset, size);
  }
}