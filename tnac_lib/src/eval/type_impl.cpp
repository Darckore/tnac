#include "eval/type_impl.hpp"

// array_data
namespace tnac::eval
{
  // Special members

  array_data::~array_data() noexcept = default;

  array_data::array_data(store& valStore, size_type prealloc) noexcept :
    m_store{ &valStore }
  {
    m_data.reserve(prealloc);
  }


  // Public members

  array_data::size_type array_data::size() const noexcept
  {
    return m_data.size();
  }

  void array_data::add(value item) noexcept
  {
    m_data.push_back(std::move(item));
  }

  store& array_data::val_store() const noexcept
  {
    return *m_store;
  }
}


// array_wrapper
namespace tnac::eval
{
  // Special members

  array_wrapper::~array_wrapper() noexcept
  {
    auto&& arr = data();
    if (!arr.is_last())
      return;

    auto&& list = arr.list();
    list.remove(arr);
  }

  array_wrapper::array_wrapper(array_data& arr, size_type offset, size_type count) noexcept :
    wrapper_base{ arr },
    m_offset{ offset },
    m_count{ count }
  {
  }

  array_wrapper::array_wrapper(array_data& arr, size_type offset) noexcept :
    array_wrapper{ arr, offset, arr.size() - offset }
  {
    UTILS_ASSERT(arr.size() >= offset);
  }

  array_wrapper::array_wrapper(array_data& arr) noexcept :
    array_wrapper{ arr, {}, arr.size() }
  {
  }


  // Public members

  array_wrapper::wrapper_base::reference array_wrapper::data() noexcept
  {
    return FROM_CONST(data);
  }

  array_wrapper::size_type array_wrapper::offset() const noexcept
  {
    return m_offset;
  }

  array_wrapper::size_type array_wrapper::size() const noexcept
  {
    return m_count;
  }

  entity_id array_wrapper::id() const noexcept
  {
    return &data();
  }

  store& array_wrapper::val_store() const noexcept
  {
    return data().val_store();
  }


  // Private members

  array_wrapper::wrapper_base::const_reference array_wrapper::data() const noexcept
  {
    return *operator->();
  }

  array_wrapper::size_type array_wrapper::calc_begin() const noexcept
  {
    UTILS_ASSERT(m_offset <= data().size());
    return m_offset;
  }
  array_wrapper::size_type array_wrapper::calc_end() const noexcept
  {
    const auto res = m_offset + m_count;
    UTILS_ASSERT(res <= data().size());
    return res;
  }
  array_wrapper::size_type array_wrapper::calc_rbegin() const noexcept
  {
    return data().size() - calc_end();
  }
  array_wrapper::size_type array_wrapper::calc_rend() const noexcept
  {
    return data().size() - calc_begin();
  }
}