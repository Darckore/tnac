//
// Value storage
//

#pragma once

namespace tnac::eval
{
  class array_data;
  class array_wrapper;
}

namespace tnac::eval
{
  //
  // Stores instances of various supported types
  //
  class store final
  {
  public:
    using array_list  = utils::ilist<array_data>;
    using array_wraps = utils::ilist<array_wrapper>;
    using size_type   = std::size_t;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(store);

    ~store() noexcept;

    store() noexcept;

  public:
    //
    // Allocates a new empty array of the given size
    //
    array_data& allocate_array(size_type size) noexcept;

    //
    // Allocates an array and wraps it with zero offset and the specified size
    //
    array_wrapper& alloc_wrapped(size_type size) noexcept;

    //
    // Wraps the given array with the specified offset and length
    //
    array_wrapper& wrap(array_data& arr, size_type offset, size_type size) noexcept;

    //
    // Wraps the given array with the specified offset and its remaining tail size
    //
    array_wrapper& wrap(array_data& arr, size_type offset) noexcept;

    //
    // Wraps the entire array
    //
    array_wrapper& wrap(array_data& arr) noexcept;

    //
    // Creates a wrapper from an existing one, but with a different offset and size
    //
    array_wrapper& wrap(array_wrapper& aw, size_type offset, size_type size) noexcept;

  private:
    array_list  m_arrData{};
    array_wraps m_arrWrappers{};
  };
}