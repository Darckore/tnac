#pragma once
#include <complex>
#include "utils/utils.hpp"

namespace tnac
{
  using buf_t    = std::string;
  using string_t = std::string_view;
  using char_t   = string_t::value_type;

  using file_data = std::expected<string_t, std::error_code>;
}


#define TNAC_BAD_PLATFORM static_assert(false, "Platform not supported");

#if _WIN64
#define TNAC_WINDOWS 1
#else
#endif

namespace tnac::rt
{
  using in_stream  = utils::istream;
  using out_stream = utils::ostream;
}

namespace tnac
{
  //
  // Helper object to facilitate easy casts from pointers to entity ids
  //
  class entity_id final
  {
  public:
    using id_t = std::uintptr_t;

  public:
    CLASS_SPECIALS_ALL_CUSTOM(entity_id);

    static consteval auto invalid_id() noexcept { return ~id_t{}; }

    constexpr entity_id() noexcept :
      m_value{ invalid_id() }
    {}

    constexpr entity_id(id_t id) noexcept :
      m_value{ id }
    {}

    entity_id(const void* ent) noexcept :
      entity_id{ reinterpret_cast<id_t>(ent) }
    {}

    constexpr entity_id(std::nullptr_t) noexcept :
      entity_id{ id_t{} }
    {}

    constexpr auto operator*() const noexcept
    {
      return m_value;
    }

    constexpr bool operator==(const entity_id&) const noexcept = default;

  private:
    id_t m_value{};
  };

  //
  // Defines an object visitable by the specified visitor
  //
  template <typename Obj, typename Visitor>
  concept visitable = requires(Visitor v, Obj* n)
  {
    v.visit(*n);
  };

  //
  // Defines an object previewable by the specified visitor
  //
  template <typename Obj, typename Visitor>
  concept previewable = requires(Visitor v, Obj* n)
  {
    { v.preview(*n) }->std::same_as<bool>;
  };
}

template <>
struct std::hash<tnac::entity_id>
{
  constexpr auto operator()(const tnac::entity_id& id) const noexcept
  {
    return *id;
  }
};


namespace tnac
{
  //
  // Defines a counter type
  //
  template <typename T>
  concept counter =
    utils::integer<T> &&
    std::unsigned_integral<T>;

  //
  // Defines a ref-counted type capable of deleting itself
  //
  template <typename T>
  concept self_deletable = requires (T&& obj)
  {
    { obj.remove() };
  };

  //
  // Base for ref-counted types
  //
  template <typename Derived, counter C = std::size_t>
  class ref_counted
  {
  public:
    using counted_base = ref_counted<Derived, C>;
    using object_type  = Derived;
    using counter_type = C;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(ref_counted);

    ~ref_counted() noexcept = default;

    ref_counted() noexcept = default;

  public:
    //
    // Adds a reference to the current object
    //
    void addref() noexcept
    {
      ++m_refs;
    }

    //
    // Removes a reference for the current object
    //
    void release() noexcept
    {
      if(m_refs)
        --m_refs;
    }

    //
    // Checks whether the object has any live reference
    //
    bool hasref() const noexcept
    {
      return static_cast<bool>(m_refs);
    }

  private:
    counter_type m_refs{};
  };

  //
  // Defines a type derived from ref_counted
  //
  template <typename T>
  concept ref_object = std::derived_from<T, ref_counted<T>>;

  //
  // A wrapper for a ref-counted type
  // Holds a pointer to the actual object and manages its ref counter
  //
  template <typename Object>
  class rc_wrapper
  {
  public:
    using object_type     = Object;
    using pointer         = object_type*;
    using const_pointer   = const object_type*;
    using reference       = object_type&;
    using const_reference = const object_type&;

  private:
    static void ref(pointer obj) noexcept
    {
      if (obj)
        obj->addref();
    }
    static void unref(pointer obj) noexcept
    {
      if (obj)
        obj->release();

      if constexpr (self_deletable<object_type>)
      {
        if (!obj || obj->hasref())
          return;

        obj->remove();
      }
    }

  public:
    rc_wrapper() noexcept = delete;

    explicit rc_wrapper(reference obj) noexcept :
      m_obj{ &obj }
    {
      static_assert(ref_object<Object>);
      obj.addref();
    }

    ~rc_wrapper() noexcept
    {
      unref(m_obj);
    }

    rc_wrapper(const rc_wrapper& other) noexcept
    {
      unref(m_obj);
      m_obj = other.m_obj;
      ref(m_obj);
    }

    rc_wrapper& operator=(const rc_wrapper& other) noexcept
    {
      if (this == &other)
        return *this;

      unref(m_obj);
      m_obj = other.m_obj;
      ref(m_obj);
      return *this;
    }

    rc_wrapper(rc_wrapper&& other) noexcept
    {
      unref(m_obj);
      m_obj = other.m_obj;
      other.m_obj = {};
    }

    rc_wrapper& operator=(rc_wrapper&& other) noexcept
    {
      if (this == &other)
        return *this;

      unref(m_obj);
      m_obj = other.m_obj;
      other.m_obj = {};
      return *this;
    }

  public:
    //
    // Checks whether this wrapper references an object
    //
    explicit operator bool() const noexcept
    {
      return static_cast<bool>(m_obj);
    }

    //
    // Provides access to the underlying object
    // Must check whether is valid first
    //
    const_pointer operator->() const noexcept
    {
      return m_obj;
    }

    //
    // Provides access to the underlying object
    // Must check whether is valid first
    //
    pointer operator->() noexcept
    {
      return FROM_CONST(operator->);
    }

  private:
    pointer m_obj{};
  };
}

//
// A macro to auto-generate samy remove functions for various ref-conted types
//
#define SELF_DELETE() \
UTILS_ASSERT(!hasref()); \
SCOPE_GUARD(auto&& lst = list(); lst.remove(*this))