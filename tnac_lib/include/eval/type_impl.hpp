//
// Internal supporting parts of types
//

#pragma once

#include "eval/value.hpp"

namespace tnac::eval
{
  //
  // Array underlying data
  //
  class array_data final :
    public ref_counted<array_data>,
    public utils::ilist_node<array_data>
  {
  public:
    using data_type = std::vector<value>;
    using size_type = data_type::size_type;

  public:
    CLASS_SPECIALS_NONE(array_data);

    ~array_data() noexcept;

    explicit array_data(size_type prealloc) noexcept;

  public:
    //
    // Returns the current size of the stored data
    //
    size_type size() const noexcept;

    //
    // Appends a value to the stored data
    //
    void add(value item) noexcept;

  public:
    auto begin() const noexcept
    {
      return m_data.begin();
    }
    auto begin() noexcept
    {
      return m_data.begin();
    }
    auto cbegin() const noexcept
    {
      return m_data.cbegin();
    }
    auto rbegin() const noexcept
    {
      return m_data.rbegin();
    }
    auto rbegin() noexcept
    {
      return m_data.rbegin();
    }
    auto crbegin() const noexcept
    {
      return m_data.crbegin();
    }

    auto end() const noexcept
    {
      return m_data.end();
    }
    auto end() noexcept
    {
      return m_data.end();
    }
    auto cend() const noexcept
    {
      return m_data.cend();
    }
    auto rend() const noexcept
    {
      return m_data.rend();
    }
    auto rend() noexcept
    {
      return m_data.rend();
    }
    auto crend() const noexcept
    {
      return m_data.crend();
    }

  private:
    data_type m_data;
  };


  //
  // Array wrapper
  // Holds an array data pointer, an offset, and item count
  //
  class array_wrapper final :
    public ref_counted<array_wrapper>,
    public rc_wrapper<array_data>,
    public utils::ilist_node<array_wrapper>
  {
  public:
    using size_type    = array_data::size_type;
    using wrapper_base = rc_wrapper<array_data>;

  public:
    CLASS_SPECIALS_NONE(array_wrapper);

    ~array_wrapper() noexcept;

    array_wrapper(array_data& arr, size_type offset, size_type count) noexcept;

    array_wrapper(array_data& arr, size_type offset) noexcept;

    explicit array_wrapper(array_data& arr) noexcept;

  public:
    //
    // A dirty-dirty hack to access underlying data
    // Not a huge issue since we won't be using the thing in non-const contents
    //
    wrapper_base::reference data() noexcept;

    //
    // Returns the size of data available to the wrapper
    //
    size_type size() const noexcept;

  public:
    auto begin() const noexcept
    {
      return data().begin();
    }
    auto cbegin() const noexcept
    {
      return data().cbegin();
    }
    auto rbegin() const noexcept
    {
      return data().rbegin();
    }
    auto crbegin() const noexcept
    {
      return data().crbegin();
    }

    auto end() const noexcept
    {
      return data().end();
    }
    auto cend() const noexcept
    {
      return data().cend();
    }
    auto rend() const noexcept
    {
      return data().rend();
    }
    auto crend() const noexcept
    {
      return data().crend();
    }

  private:
    wrapper_base::const_reference data() const noexcept;

  private:
    size_type m_offset{};
    size_type m_count{};
  };
}